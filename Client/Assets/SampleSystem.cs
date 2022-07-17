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

	const int recvFullsize = 1024;
	public byte[] recvBuffer = new byte[recvFullsize];

	void Awake()
	{
		serverAddress = new(IPAddress.Loopback, 9000);
	}

	void Start()
	{
		try
		{
			tcpClient.Connect(serverAddress);
		}
		catch (Exception e)
		{
			Debug.LogError(e.Message);
			tcpClient.Close();
		}

		tcpStream = tcpClient.GetStream();
		if (tcpStream is null)
		{
			throw new Exception("소켓 연결 실패!");
		}

		tcpStream.BeginRead(recvBuffer, 0, recvBuffer.Length, CallbackRead, null);
	}

	void Update()
	{
		//if (tcpClient != null && tcpClient.Connected)
		{ }
	}

	void CallbackRead(IAsyncResult result)
	{
		var byte_recv = tcpStream.EndRead(result);

		//if (result.IsCompleted)
		if (0 < byte_recv)
		{

		}
		else
		{

		}

		tcpStream.EndRead(result);
	}
}
