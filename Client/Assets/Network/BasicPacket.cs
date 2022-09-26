using System;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.Runtime.InteropServices;

using UnityEngine;

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct BasicPacket
{
	public Protocols myProtocol;
	public short mySize;
}
