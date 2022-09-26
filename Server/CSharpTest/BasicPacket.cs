using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace CSharpTest
{
	[StructLayout(LayoutKind.Sequential, Pack = 1, Size = 12)]
	internal class BasicPacket
	{
		public Protocol myProtocol;
		public Int32 mySize;
	}
}
