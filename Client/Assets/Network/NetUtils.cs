using System;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.Runtime.InteropServices;

[Serializable]
public static class NetUtils
{
	public static int SizeOf<T>() where T : class => Marshal.SizeOf(typeof(T));
	public static int SizeOf<T>(Type type) => Marshal.SizeOf<Type>(type);
	public static int SizeOf<T>(T obj) where T : class => Marshal.SizeOf(obj);
	public static int SizeOf<T>(in T obj) where T : struct => Marshal.SizeOf(obj);
	public static byte[] Serialize<T>(in T target, int size) where T : class
	{
		byte[] result = new byte[size];

		var handle = Marshal.AllocHGlobal(size);
		Marshal.StructureToPtr(target, handle, false);
		Marshal.Copy(handle, result, 0, size);
		Marshal.FreeHGlobal(handle);

		return result;
	}
	public static T Deserialize<T>(in byte[] buffer, int size) where T : class
	{
		return Parse<T>(in buffer, size);
	}
	public static byte[] Serialize<T>(in T target) where T : class
	{
		return Serialize(in target, SizeOf(target));
	}
	public static T Deserialize<T>(in byte[] buffer) where T : class
	{
		return Deserialize<T>(in buffer, SizeOf(typeof(T)));
	}
	public static bool TrySerialize<T>(in T target, out byte[] buffer) where T : class
	{
		buffer = null;

		int size = SizeOf(typeof(T));
		if (buffer is null || buffer.Length < size)
		{
			return false;
		}

		buffer = Serialize<T>(in target);

		return true;
	}
	public static bool TryDeserialize<T>(in byte[] buffer, out T result) where T : class
	{
		result = null;

		int size = SizeOf(typeof(T));
		if (buffer is null || buffer.Length < size)
		{
			return false;
		}

		result = Deserialize<T>(in buffer, size);

		return true;
	}
	public static bool PokeBoolean(in byte[] buffer, int offset = 0)
	{
		return BitConverter.ToBoolean(buffer, offset);
	}
	public static char PokeChar(in byte[] buffer, int offset = 0)
	{
		return BitConverter.ToChar(buffer, offset);
	}
	public static int PokeInt(in byte[] buffer, int offset = 0)
	{
		return BitConverter.ToInt32(buffer, offset);
	}
	public static long PokeLong(in byte[] buffer, int offset = 0)
	{
		return BitConverter.ToInt64(buffer, offset);
	}
	public static float PokeFloat(in byte[] buffer, int offset = 0)
	{
		return BitConverter.Int32BitsToSingle(PokeInt(in buffer, offset));
	}
	public static double PokeDouble(in byte[] buffer, int offset = 0)
	{
		return BitConverter.ToDouble(buffer, offset);
	}

	public static T Parse<T>(in byte[] buffer, int size) where T : class
	{
		var ptr = Marshal.AllocHGlobal(size);
		Marshal.Copy(buffer, 0, ptr, size);

		var result = Marshal.PtrToStructure<T>(ptr);
		Marshal.FreeHGlobal(ptr);

		return result;
	}
	public static T Parse<T>(in Span<byte> buffer) where T : class
	{
		return Parse<T>(buffer.ToArray(), buffer.Length);
	}
}
