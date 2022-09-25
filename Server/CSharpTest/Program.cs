using System.Diagnostics;
using System.Net.Sockets;
using System.Net;
using System.Runtime.InteropServices;
using System.ComponentModel;

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

		public bool userLoginned;
		public string userName;
		public string userPassword;

		static void Main(string[] args)
		{
			Console.WriteLine("Hello, World!");

			Program program = new();

			if (program.Connect())
			{
				Console.WriteLine("연결 완료");

				string? temp_username, temp_password;
				while (true)
				{
					Console.Write("사용자 ID 입력: ");
					temp_username = Console.ReadLine();

					if (temp_username is null)
					{
						continue;
					}

					break;
				}

				while (true)
				{
					Console.Write("사용자 비밀번호 입력: ");
					temp_password = Console.ReadLine();

					if (temp_password is null)
					{
						continue;
					}

					break;
				}

				byte[] packet = new byte[256];

				Parser.ParseStruct<BasicPacket>(packet);


			}

			while (true)
			{
				if (program.userLoginned)
				{
					break;
				}
			}
		}

		public Program()
		{
			userLoginned = false;
			userName = new("");
			userPassword = new("");
		}

		public void OnDestroy()
		{
			if (tcpClient is not null && tcpClient.Connected)
			{
				tcpClient.Close();
			}
		}
		public bool Connect()
		{
			Console.WriteLine("서버 접속 중...");

			myClient = new(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

			serverAddress = new(IPAddress.Loopback, Server.Port);

			myClient.Connect(serverAddress);

			if (myClient is not null && myClient.Connected)
			{
				Console.WriteLine("서버 접속 성공함.");

				ReceiveForSelf(0, BUFFSIZE);
			}
			else
			{
				Console.WriteLine("TCP 소켓의 연결 실패");
				return false;
			}

			return true;
		}

		private void OnSend(IAsyncResult result)
		{
			if (myClient is null || !myClient.Connected)
			{
				Console.WriteLine("TCP 송신 불가능!");
				return;
			}

			var byte_sent = myClient.EndSend(result, out SocketError error_state);
			Console.WriteLine("TCP 소켓으로 서버로 " + byte_sent + "바이트를 줌.");
			Console.WriteLine("TCP 소켓 상태: " + error_state);

			if (byte_sent <= 0)
			{
				throw new System.Exception("TCP 오류!");
			}
			else if (result.IsCompleted)
			{

			}
			else
			{

			}
		}
		public IAsyncResult SendBuffer(in byte[] buffer)
		{
			return myClient.BeginSend(buffer, 0, buffer.Length, SocketFlags.None, OnSend, null);
		}
		public IAsyncResult SendPacket<T>(in T packet) where T : BasicPacket
		{
			var buffer = Parser.ParseBytes(packet);

			if (buffer is not null)
			{
				return SendBuffer(buffer);
			}
			else
			{
				throw new ContextMarshalException("패킷 변환 오류!");
			}
		}
		public IAsyncResult SendPacketFromByte(in byte[] data)
		{
			return myClient.BeginSend(data, 0, data.Length, SocketFlags.None, OnSend, null);
		}
		private void OnReceive(IAsyncResult result)
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

			ReceiveForSelf(tcpReceived, BUFFSIZE - tcpReceived);
		}
		private IAsyncResult ReceiveForSelf(int offset, int size)
		{
			return myClient.BeginReceive(recvBuffer, offset, size, SocketFlags.None, OnReceive, null);
		}
		private BasicPacket? Filter()
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
