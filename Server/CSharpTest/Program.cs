using System.Diagnostics;
using System.Net.Sockets;
using System.Net;

namespace CSharpTest
{
	internal class Program
	{
		public IPEndPoint? serverAddress;
		public TcpClient? tcpClient;
		public UdpClient? udpClient;
		public NetworkStream? tcpStream;
		public NetworkStream? udpStream;

		public int tcpReceived = 0;

		public const int BUFFSIZE = 1024;
		public byte[] recvBuffer = new byte[BUFFSIZE];

		static void Main(string[] args)
		{
			Console.WriteLine("Hello, World!");

			Program program = new();

			program.Connect();

			while (true) ;
		}

		void OnDestroy()
		{
			if (tcpClient is not null && tcpClient.Connected)
			{
				tcpClient.Close();
			}
		}
		void Connect()
		{
			Console.WriteLine("서버 접속 중...");

			serverAddress = new(IPAddress.Loopback, 9000);
			tcpClient = new(serverAddress);

			try
			{
				tcpClient.Connect(serverAddress);
			}
			catch (Exception e)
			{
				Console.WriteLine(e.Message);
				tcpClient.Close();
			}

			if (tcpClient is not null && tcpClient.Connected)
			{
				Console.WriteLine("서버 접속 성공함.");

				tcpStream = tcpClient.GetStream();
				if (tcpStream is null)
				{
					throw new Exception("소켓 연결 실패!");
				}

				RecvTCP(0, BUFFSIZE);
			}
			else
			{
				Console.WriteLine("TCP 소켓 생성 실패");
			}
		}
		void CallbackRead(IAsyncResult result)
		{
			if (!tcpStream.CanRead)
			{
				Console.WriteLine("TCP 수신 불가능!");
				return;
			}

			var byte_recv = tcpStream.EndRead(result);
			Console.WriteLine("TCP 소켓으로 서버로부터 " + byte_recv + "바이트를 받음.");

			//if (result.IsCompleted)
			if (0 < byte_recv)
			{
				if (BitConverter.IsLittleEndian)
				{
					Array.Reverse(recvBuffer[(tcpReceived)..(tcpReceived + byte_recv)]);
				}

				tcpReceived += byte_recv;

				if (4 <= tcpReceived)
				{
					var temp_type = BitConverter.ToInt32(recvBuffer.AsSpan()[0..4]);
					var packet_type = (Protocol)(temp_type);

					if (8 <= tcpReceived)
					{
						var packet_size = BitConverter.ToInt32(recvBuffer[4..8]);

						if (packet_size <= tcpReceived)
						{
							if (BUFFSIZE <= packet_size)
							{
								Console.WriteLine("이상한 패킷을 받음!");
							}

							// 패킷 처리
							Console.WriteLine("패킷을 받음: " + packet_type.ToString());

							tcpReceived -= packet_size;

							ShiftLeft(recvBuffer, packet_size);
						}
						else
						{
							var lack = packet_size - tcpReceived;
							Console.WriteLine("TCP 소켓으로부터 받은 바이트가 " + lack + "만큼 부족함.");
						}
					}
					else
					{
						Console.WriteLine("TCP 소켓으로부터 받은 바이트가 부족함.");
					}
				}
				else
				{
					Console.WriteLine("TCP 소켓으로부터 받은 바이트가 부족해서 패킷의 종류를 알 수 없음.");
				}
			}
			else
			{
				Console.WriteLine("TCP 소켓으로부터 0바이트를 받음.");
			}

			RecvTCP(tcpReceived, BUFFSIZE - tcpReceived);
		}
		IAsyncResult RecvTCP(int offset, int size)
		{
			return tcpStream.BeginRead(recvBuffer, offset, size, CallbackRead, null);
		}
		public static void ShiftLeft(byte[] lst, int shifts)
		{
			for (int i = shifts; i < lst.Length; i++)
			{
				lst[i - shifts] = lst[i];
			}

			for (int i = lst.Length - shifts; i < lst.Length; i++)
			{
				lst[i] = 0;
			}
		}
	}
}
