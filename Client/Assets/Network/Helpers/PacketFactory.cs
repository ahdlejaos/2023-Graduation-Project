using System;
using System.Collections;
using System.Collections.Generic;

using UnityEngine;

[Serializable]
public static class PacketFactory
{
	public static BasicPacket MakePacket(Protocols protocol, short size)
	{
		return new BasicPacket()
		{
			myProtocol = protocol,
			mySize = size
		};
	}
}
