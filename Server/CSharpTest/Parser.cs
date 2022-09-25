using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace CSharpTest
{
	internal class Parser
	{
		public static byte[] ParseBytes<T>(in T target) where T : BasicPacket
		{
			//int sz = Marshal.SizeOf(typeof(T));
			int sz = Marshal.SizeOf(target);

			byte[] result = new byte[sz];

			var handle = Marshal.AllocHGlobal(sz);
			Marshal.StructureToPtr(target, handle, false);
			Marshal.Copy(handle, result, 0, sz);
			Marshal.FreeHGlobal(handle);

			return result;
		}

		public static T? ParseStruct<T>(in byte[] buffer) where T : BasicPacket
		{
			int sz = Marshal.SizeOf(typeof(T));
			if (buffer is null || buffer.Length < sz)
			{
				return null;
			}

			var ptr = Marshal.AllocHGlobal(sz);
			Marshal.Copy(buffer, 0, ptr, sz);

			var result = Marshal.PtrToStructure<T>(ptr);
			Marshal.FreeHGlobal(ptr);

			return result;
		}

		public const int ptrProtocol = 0;
		public const int ptrPacketkSize = sizeof(Protocol);
		public const int ptrSpecial = ptrPacketkSize + sizeof(int);
	}
}
