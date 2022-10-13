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

	public static void Main(int argc, char[] argv)
	{

	}

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
		if (IsDisconnected())
		{
			Debug.LogError("TCP ���� �Ұ���!");
			return;
		}

		int bytes = myConnector.EndReceive(result);
		Debug.Log("TCP �������� �����κ��� " + bytes + "����Ʈ�� ����.");

		int buffer_last_offset = myBuffer.ApplyReceived(bytes);

		if (NetConstants.szPkMinimum <= buffer_last_offset)
		{
			ref byte[] data = ref myBuffer.GetData();

			var tempch = BitConverter.ToChar(data[0..2]);
			var packet_type = (Protocols) (tempch);

			var tempst = BitConverter.ToInt16(data[2..4]);
			var packet_size = (short) (tempst);

			if (packet_size <= buffer_last_offset)
			{
				// ��Ŷ ó��
				Debug.Log("��Ŷ�� ����: " + packet_type.ToString());
				
				var ok = myBuffer.TryRead<BasicPacket>(out var packet);
				Debug.Log("���� ��Ŷ: " + packet);

				switch (packet_type)
				{
					case Protocols.SC_SERVER_INFO:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_SIGNIN_SUCCESS:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_SIGNIN_FAILURE:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_SIGNUP_SUCCESS:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_SIGNUP_FAILURE:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_RESPOND_ROOMS:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_RESPOND_USERS:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_RESPOND_VERSION:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_ROOM_CREATED:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_GAME_START:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_CREATE_PLAYER:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_CREATE_ENTITY:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_CREATE_OBJET:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_MOVE_CHARACTER:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_MOVE_OBJET:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_UPDATE_CHARACTER:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_UPDATE_OBJET:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_REMOVE_CHARACTER:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_REMOVE_OBJET:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;

					case Protocols.SC_CHAT:
					{
						//ok = myBuffer.TryRead<PacketLogin>(out var packet);
					}
					break;
				}
			}
			else
			{
				var lack = packet_size - buffer_last_offset;
				Debug.Log("TCP �������κ��� ���� ����Ʈ�� ���Ͽ��� " + lack + "��ŭ ������.");
			}
		}
		else
		{
			Debug.Log("TCP �������κ��� ���� ����Ʈ�� �����ؼ� ��Ŷ�� ������ �� ����.");
		}

		if (result.IsCompleted)
		{
		}
		else
		{
		}

		myConnector.Receive(ref myBuffer);
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
