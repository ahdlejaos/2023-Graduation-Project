using System.Diagnostics;
using System.Net.Sockets;
using System.Net;
using System.Runtime.InteropServices;

namespace CSharpTest
{
	internal class Program
	{
		public IPEndPoint? serverAddress;
		public TcpClient? tcpClient;
		public UdpClient? udpClient;
		public Socket? myClient = null;

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

			myClient = new(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

			serverAddress = new(IPAddress.Loopback, Server.Port);

			myClient.Connect(serverAddress);

			if (myClient is not null && myClient.Connected)
			{
				Console.WriteLine("서버 접속 성공함.");

				RecvTCP(0, BUFFSIZE);
			}
			else
			{
				Console.WriteLine("TCP 소켓의 연결 실패");
			}
		}
		void CallbackRead(IAsyncResult result)
		{
			if (myClient is null || !myClient.Connected)
			{
				Console.WriteLine("TCP 수신 불가능!");
				return;
			}

			var test_reads = myClient.Available;
			Console.WriteLine("받을 수 있는 바이트의 수: {0}", test_reads);

			var byte_recv = myClient.EndReceive(result, out SocketError error_state);
			Console.WriteLine("TCP 소켓으로 서버로부터 " + byte_recv + "바이트를 받음.");
			Console.WriteLine("TCP 소켓 상태: " + error_state);

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
					var packet = Filter();

					if (packet is not null)
					{

					}
					else
					{

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
			return myClient.BeginReceive(recvBuffer, offset, size, SocketFlags.None, CallbackRead, null);
		}
		BasicPacket? Filter()
		{
			BasicPacket? result = null;

			var buffer = recvBuffer.AsSpan<byte>();
			var temp_type = BitConverter.ToInt32(buffer[0..4]);
			var packet_type = (Protocol)(temp_type);

			if (8 <= tcpReceived)
			{
				var packet_size = BitConverter.ToInt32(buffer[4..8]);

				if (packet_size <= tcpReceived)
				{
					if (BUFFSIZE <= packet_size)
					{
						Console.WriteLine("이상한 패킷을 받음!");
					}

					// 패킷 처리
					Console.WriteLine("패킷을 받음: " + packet_type.ToString());

					result = new()
					{
						myProtocol = packet_type,
						mySize = packet_size
					};

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

			return result;
		}

		public static void ShiftLeft(Span<byte> lst, int shifts)
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
