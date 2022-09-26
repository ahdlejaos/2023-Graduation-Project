using System;
using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Net;

using UnityEngine;

public class NetManager : MonoBehaviour
{
	public const short serverPort = 12000;

	public NetConnector myConnector;

	void Awake()
	{
		myConnector = new(IPAddress.Loopback);
	}
	void Start()
	{
		
	}
	void Update()
	{

	}

	public bool Connect()
	{
		myConnector.Awake();
		myConnector.Start();

		return false;
	}
}
