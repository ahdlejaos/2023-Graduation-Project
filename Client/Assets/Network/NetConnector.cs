using System;
using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Net;

using UnityEngine;
using UnityEditor.PackageManager;

[Serializable]
public class NetConnector
{
	public bool isConnected = false;
	public bool isLoginned = false;

	public IPEndPoint serverAddress;
	public Socket tcpClient;
	public Socket udpClient;

	public NetConnector(IPAddress server_address)
	{
		serverAddress = new(server_address, NetManager.serverPort);
	}
	public void Awake()
	{
		tcpClient = new(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
		udpClient = new(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Udp);
	}
	public void Start()
	{
		tcpClient.Connect(serverAddress);

		if (tcpClient is not null && tcpClient.Connected)
		{
			Debug.LogError("서버로 접속 성공함.");
		}
		else
		{
			Debug.LogError("TCP 소켓의 연결이 실패함.");
		}
	}
}
