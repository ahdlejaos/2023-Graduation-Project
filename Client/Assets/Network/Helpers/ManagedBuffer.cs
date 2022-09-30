using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;

using UnityEngine;
using Unity.Netcode;

[Serializable]
public class ManagedBuffer
{
	public byte[] myBuffer;
	public int myBufferSize;
	public int myOffset;

	public ManagedBuffer(int cb_size)
	{
		myBuffer = new byte[cb_size];
		myBufferSize = cb_size;
		myOffset = 0;
	}

	// 
	/// <summary>
	/// ������ ���� ��ġ�κ��� ����ü�� �о�ɴϴ�.
	/// ������ �޸𸮸� �Ҵ��ϹǷ� ���� ����ϸ� �ȵ˴ϴ�.
	/// </summary>
	/// <typeparam name="T">���� ����ü �Ǵ� Ŭ������ �ڷ���</typeparam>
	/// <param name="offset">������ ��ġ</param>
	/// <returns>������ ����ü �Ǵ� Ŭ���� ��ü</returns>
	public T ReadFrom<T>(int offset) where T : class, new()
	{
		int sz = NetUtils.SizeOf(typeof(T));

		if (0 < sz && IsBoundsFrom(offset, sz))
		{
			var sublength = offset + sz;

			// �κ� �迭
			var subspan = myBuffer[offset..sublength];
			T result = NetUtils.Deserialize<T>(in subspan, sz);

			// ���� �迭
			var span = myBuffer[sublength..myBufferSize];
			myBuffer.CopyTo(span, offset);
			myOffset -= sz;

			return result;
		}
		else
		{
			return null;
		}
	}
	/// <summary>
	/// ������ �� ������ ����ü�� �о�ɴϴ�. ������ �޸𸮸� �Ҵ��ϹǷ� ���� ����ϸ� �ȵ˴ϴ�.
	/// </summary>
	/// <typeparam name="T">���� ����ü �Ǵ� Ŭ������ �ڷ���</typeparam>
	/// <returns>������ ����ü �Ǵ� Ŭ���� ��ü</returns>
	public T Peek<T>() where T : class, new()
	{
		return NetUtils.Deserialize<T>(myBuffer);
	}
	/// <summary>
	/// ������ ���� ��ġ�κ��� ����ü�� �а�, �� ���� ��ġ�� �����մϴ�.
	/// ������ �޸𸮸� �Ҵ��ϹǷ� ���� ����ϸ� �ȵ˴ϴ�.
	/// </summary>
	/// <typeparam name="T">���� ����ü �Ǵ� Ŭ������ �ڷ���</typeparam>
	/// <param name="offset">������ ��ġ</param>
	/// <param name="result">������� ������ ����</param>
	/// <returns>������ ����ü �Ǵ� Ŭ���� ��ü</returns>
	public bool TryReadFrom<T>(int offset, out T result) where T : class, new()
	{
		result = null;

		if (myOffset <= offset || offset < 0)
		{
			return false;
		}

		try
		{
			result = ReadFrom<T>(offset);

			if (result is null)
			{
				return false;
			}
		}
		catch (AccessViolationException e)
		{
			Debug.LogError("�޸� ���� ���ݼ� ���� �߻�!\n" + e);
			return false;
		}
		catch (IndexOutOfRangeException e)
		{
			Debug.LogError("�迭 ���� ���� �߻�!\n" + e);
			return false;
		}

		return true;
	}
	/// <summary>
	/// ������ ���� ��ġ�κ��� ����ü�� �а�, �� ���� ��ġ�� �����մϴ�.
	/// ������ �޸𸮸� �Ҵ��ϹǷ� ���� ����ϸ� �ȵ˴ϴ�.
	/// </summary>
	/// <typeparam name="T">���� ����ü �Ǵ� Ŭ������ �ڷ���</typeparam>
	/// <param name="result">������� ������ ����</param>
	/// <returns>������ ����ü �Ǵ� Ŭ���� ��ü</returns>
	public bool TryRead<T>(out T result) where T : class, new()
	{
		return TryReadFrom(0, out result);
	}
	public bool TryWriteTo<T>(int offset, in T placer) where T : class, new()
	{
		if (offset < 0)
		{
			return false;
		}

		int sz = NetUtils.SizeOf(typeof(T));

		if (0 < sz && IsBoundsFrom(offset, sz))
		{
			try
			{
				var result = NetUtils.Serialize(in placer, sz);
				result.CopyTo(myBuffer, offset);

				myOffset = Mathf.Max(offset + sz, myOffset);
			}
			catch (AccessViolationException e)
			{
				Debug.LogError("�޸� ���� ���ݼ� ���� �߻�!\n" + e);
				return false;
			}
			catch (IndexOutOfRangeException e)
			{
				Debug.LogError("�迭 ���� ���� �߻�!\n" + e);
				return false;
			}
		}
		else
		{
			return false;
		}

		return true;
	}

	//
	public int ApplyReceived(int bytes)
	{
		return (myOffset += bytes);
	}

	// 
	public ref byte[] GetData()
	{
		return ref myBuffer;
	}
	public byte[] GetCopiedData()
	{
		return myBuffer;
	}

	// 
	private bool IsBoundsFrom(int offset, int length)
	{
		return length + offset < myBufferSize;
	}
	private bool IsBoundsFrom(int length)
	{
		return IsBoundsFrom(myOffset, length);
	}
}
