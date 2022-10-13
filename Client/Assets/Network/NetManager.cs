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
			Debug.LogError("TCP 수신 불가능!");
			return;
		}

		int bytes = myConnector.EndReceive(result);
		Debug.Log("TCP 소켓으로 서버로부터 " + bytes + "바이트를 받음.");

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
				// 패킷 처리
				Debug.Log("패킷을 받음: " + packet_type.ToString());
				
				var ok = myBuffer.TryRead<BasicPacket>(out var packet);
				Debug.Log("받은 패킷: " + packet);

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
				Debug.Log("TCP 소켓으로부터 받은 바이트가 소켓에서 " + lack + "만큼 부족함.");
			}
		}
		else
		{
			Debug.Log("TCP 소켓으로부터 받은 바이트가 부족해서 패킷을 조립할 수 없음.");
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
			Debug.LogError("연결 오류!");
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
