using System;
using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Net;
using System.Threading.Tasks;

using UnityEngine;
using UnityEditor.PackageManager;
using UnityEngine.Events;
using System.Threading;
using UnityEditor.Profiling.Memory.Experimental;
using Unity.VisualScripting.FullSerializer.Internal;

[Serializable]
public enum NetConnectionStates
{
	None = 0,
	FirstContact, FirstRetrying, FailedAtFirst,
	Retrying, Failed,
	Connected,
	Stopped
}

[Serializable]
public class NetConnector
{
	public NetConnectionStates myOption;
	public int connectionTries = 0;
	public bool isConnected = false;
	public Thread connectionThread = null;

	public IPEndPoint serverAddress;
	public Socket tcpClient;
	public Socket udpClient;

	public AsyncCallback recvCallback;
	public AsyncCallback sendCallback;

	public NetConnector(IPAddress server_address)
	{
		myOption = NetConnectionStates.None;

		serverAddress = new(server_address, NetConstants.serverPort);
	}
	public void Awake()
	{
		tcpClient = new(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
		udpClient = new(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
	}
	public void Start()
	{
		myOption = NetConnectionStates.FirstContact;

		connectionThread = new(TryFirstConnect);
		connectionThread.Start();
		connectionThread.Join();
	}
	public void Update(float delta_time)
	{
		if (!IsStopped())
		{
			if (!IsConnected())
			{

			}
		}
		else
		{

		}
	}

	// 
	public IAsyncResult BeginReceive(ref byte[] buffer)
	{
		return Receive(ref buffer, 0);
	}
	public IAsyncResult BeginReceive(ref ManagedBuffer buffer)
	{
		return Receive(ref buffer, 0);
	}
	public IAsyncResult Receive(ref byte[] buffer, int offset, int size = NetConstants.BUFSIZ, object info = null)
	{
		return tcpClient.BeginReceive(buffer, offset, size - offset, SocketFlags.None, recvCallback, info);
	}
	public IAsyncResult Receive(ref ManagedBuffer buffer, object info = null)
	{
		return Receive(ref buffer.GetData(), buffer.myOffset, NetConstants.BUFSIZ - buffer.myOffset, info);
	}
	public int EndReceive(IAsyncResult result)
	{
		return tcpClient.EndReceive(result);
	}
	public int EndReceive(IAsyncResult result, out SocketError error)
	{
		return tcpClient.EndReceive(result, out error);
	}

	// 
	public bool IsConnected() => tcpClient is not null && tcpClient.Connected;
	public bool IsDisconnected() => tcpClient is null || !tcpClient.Connected;
	public bool IsRetryingNow() => myOption is NetConnectionStates.FirstRetrying or NetConnectionStates.Retrying;
	public bool IsFailed() => myOption is NetConnectionStates.FailedAtFirst or NetConnectionStates.Failed;
	public bool IsFailedAtFirst() => NetConnectionStates.FailedAtFirst == myOption;
	public bool IsStopped() => NetConnectionStates.Stopped == myOption;

	// 
	private void TryFirstConnect()
	{
		myOption = NetConnectionStates.FirstRetrying;

		while (true)
		{
			try
			{
				tcpClient.Connect(serverAddress);
			}
			catch (SocketException e)
			{
				myOption = NetConnectionStates.FailedAtFirst;

				Debug.LogError("처음의 TCP 소켓 연결이 실패함.\n오류: " + e);
				break;
			}

			if (IsConnected())
			{
				Debug.Log("서버로 접속 성공함.");
				break;
			}
			else
			{
				Debug.LogError("알 수 없는 이유로 인해 처음의 TCP 소켓 연결이 실패함.");
				break;
			}
		}
	}
}
