using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Net.Http;

using Unity;
using Unity.Netcode;
using Unity.VisualScripting;
using UnityEngine;
using UnityEngine.Events;

[Singleton]
public class NetManager : MonoBehaviour
{
	public NetConnector myConnector;
	public ManagedBuffer myBuffer;

	public NetUser mySelf;

	void Awake()
	{
		myConnector = new(IPAddress.Loopback);
		myBuffer = new(NetConstants.BUFSIZ);
		mySelf = new();

		AttachRecvCallback(OnReceive);
		AttachSendCallback(OnSend);
	}
	void Start()
	{
		StartCoroutine(FirstConnect());
	}
	void Update()
	{
		var delta_time = Time.deltaTime;

		myConnector.Update(delta_time);
	}

	// 
	public void OnReceive(IAsyncResult result)
	{
		if (result.IsCompleted)
		{

		}
		else
		{

		}


		if (!TcpClient.CanRead)
		{
			Debug.LogError("TCP ���� �Ұ���!");
			return;
		}

		var byte_recv = tcpStream.EndRead(result);
		Debug.Log("TCP �������� �����κ��� " + byte_recv + "����Ʈ�� ����.");

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
				var packet_type = (Protocols) (temp_type);

				if (NetConstants.szPkMinimum <= tcpReceived)
				{
					var packet_size = BitConverter.ToInt32(recvBuffer[4..8]);

					if (packet_size <= tcpReceived)
					{
						if (NetConstants.BUFSIZ <= packet_size)
						{
							Debug.LogError("�̻��� ��Ŷ�� ����!");
						}

						// ��Ŷ ó��
						Debug.Log("��Ŷ�� ����: " + packet_type.ToString());

						tcpReceived -= packet_size;

						ShiftLeft(recvBuffer, packet_size);
					}
					else
					{
						var lack = packet_size - tcpReceived;
						Debug.Log("TCP �������κ��� ���� ����Ʈ�� " + lack + "��ŭ ������.");
					}
				}
				else
				{
					Debug.Log("TCP �������κ��� ���� ����Ʈ�� ������.");
				}
			}
			else
			{
				Debug.Log("TCP �������κ��� ���� ����Ʈ�� �����ؼ� ��Ŷ�� ������ �� �� ����.");
			}
		}
		else
		{
			Debug.Log("TCP �������κ��� 0����Ʈ�� ����.");
		}

		RecvTCP(tcpReceived, NetConstants.BUFSIZ - tcpReceived);
	}
	public void OnSend(IAsyncResult result)
	{
		if (result.IsCompleted)
		{

		}
		else
		{

		}
	}

	// 
	public IEnumerator FirstConnect()
	{
		if (IsConnected())
		{
			goto Done;
		}

		myConnector.Awake();
		yield return new WaitForSeconds(0.1f);

		myConnector.Start();
		yield return new WaitForSeconds(0.1f);

		while (IsRetryingNow())
		{
			yield return new WaitForSeconds(0.1f);
		};

		if (IsConnected())
		{
			myConnector.BeginReceive(ref myBuffer);
		}
		else
		{
			Debug.LogError("���� ����!");
			yield break;
		}

	Done:
		yield break;
	}

	// 
	public void SetRecvCallback(in AsyncCallback callback)
	{
		myConnector.recvCallback = callback;
	}
	public void AttachRecvCallback(in AsyncCallback callback)
	{
		myConnector.recvCallback += callback;
	}
	public void SetSendCallback(in AsyncCallback callback)
	{
		myConnector.sendCallback = callback;
	}
	public void AttachSendCallback(in AsyncCallback callback)
	{
		myConnector.sendCallback += callback;
	}

	// 
	public bool IsConnected() => myConnector.IsConnected();
	public bool IsDisconnected() => myConnector.IsDisconnected();
	public bool IsRetryingNow() => myConnector.IsRetryingNow();
	public bool IsFailed() => myConnector.IsFailed();
	public bool IsFailedAtFirst() => myConnector.IsFailedAtFirst();
	public bool IsStopped() => myConnector.IsStopped();
}
