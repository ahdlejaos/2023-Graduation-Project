using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using System;
using System.Net.Sockets;
using System.Net;
using System.Net.Http;

public class SampleSystem : MonoBehaviour
{
	public IPEndPoint serverAddress;
	public TcpClient tcpClient;
	public UdpClient udpClient;
	public NetworkStream tcpStream;
	public NetworkStream udpStream;

	public int tcpReceived = 0;

	public byte[] recvBuffer = new byte[NetConstants.BUFSIZ];

	void Awake()
	{
	}
	void Start()
	{
		Connect();
	}
	void Update()
	{
		//if (tcpClient != null && tcpClient.Connected)
		{ }
	}
	void OnDestroy()
	{
		if (tcpClient is not null && tcpClient.Connected)
		{
			tcpClient.Close();
		}
	}

	IEnumerator CheckConnection()
	{
		while (true)
		{

			yield return new WaitForEndOfFrame();
		}
	}
	void Connect()
	{
		Debug.Log("서버 접속 중...");

		serverAddress = new(IPAddress.Loopback, 9000);
		tcpClient = new(serverAddress);

		try
		{
			tcpClient.Connect(serverAddress);
		}
		catch (Exception e)
		{
			Debug.LogError(e.Message);
			tcpClient.Close();
		}

		if (tcpClient is not null && tcpClient.Connected)
		{
			Debug.Log("서버 접속 성공함.");

			tcpStream = tcpClient.GetStream();
			if (tcpStream is null)
			{
				throw new Exception("소켓 연결 실패!");
			}

			RecvTCP(0, NetConstants.BUFSIZ);
		}
		else
		{
			Debug.LogError("TCP 소켓 생성 실패");
		}
	}
	void CallbackRead(IAsyncResult result)
	{
		if (!tcpStream.CanRead)
		{
			Debug.LogError("TCP 수신 불가능!");
			return;
		}

		var byte_recv = tcpStream.EndRead(result);
		Debug.Log("TCP 소켓으로 서버로부터 " + byte_recv + "바이트를 받음.");

		//if (result.IsCompleted)
		if (0 < byte_recv)
		{
			if (BitConverter.IsLittleEndian)
			{
				Array.Reverse(recvBuffer[(tcpReceived)..(tcpReceived + byte_recv)]);
			}

			tcpReceived += byte_recv;

			if (NetConstants.szPkProtocol <= tcpReceived)
			{
				var temp_type = BitConverter.ToInt32(recvBuffer[0..4]);
				var packet_type = (Protocols)(temp_type);

				if (NetConstants.szPkMinimum <= tcpReceived)
				{
					var packet_size = BitConverter.ToInt32(recvBuffer[4..8]);

					if (packet_size <= tcpReceived)
					{
						if (NetConstants.BUFSIZ <= packet_size)
						{
							Debug.LogError("이상한 패킷을 받음!");
						}

						// 패킷 처리
						Debug.Log("패킷을 받음: " + packet_type.ToString());

						tcpReceived -= packet_size;

						ShiftLeft(recvBuffer, packet_size);
					}
					else
					{
						var lack = packet_size - tcpReceived;
						Debug.Log("TCP 소켓으로부터 받은 바이트가 " + lack + "만큼 부족함.");
					}
				}
				else
				{
					Debug.Log("TCP 소켓으로부터 받은 바이트가 부족함.");
				}
			}
			else
			{
				Debug.Log("TCP 소켓으로부터 받은 바이트가 부족해서 패킷의 종류를 알 수 없음.");
			}
		}
		else
		{
			Debug.Log("TCP 소켓으로부터 0바이트를 받음.");
		}

		RecvTCP(tcpReceived, NetConstants.BUFSIZ - tcpReceived);
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
