using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace CSharpTest
{
	[StructLayout(LayoutKind.Sequential, Pack = 1)]
	internal class BasicPacket
	{
		public Protocol myProtocol;
		public Int32 mySize;

		public const int ptrProtocol = 0;
		public const int ptrPacketkSize = sizeof(Protocol);
		public const int ptrSpecial = ptrPacketkSize + sizeof(int);
	}
}
