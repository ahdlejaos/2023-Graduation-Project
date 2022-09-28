using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[Serializable]
public static class NetConstants // ScriptableObject
{
	public const short serverPort = 12000;
	public const short serverPacketSize = 12000;

	public const int BUFSIZ = 512;

	public const int szPkProtocol = sizeof(Protocols);
	public const int szPkSize = sizeof(short);
	public const int szPkMinimum = szPkProtocol + szPkSize;

	public const int ptrProtocol = 0;
	public const int ptrPacketSize = szPkProtocol;
	public const int ptrSpecial = ptrPacketSize + szPkSize;
}
