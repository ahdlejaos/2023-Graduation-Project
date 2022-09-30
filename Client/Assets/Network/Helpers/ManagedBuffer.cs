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
	/// 지정한 버퍼 위치로부터 구조체를 읽어옵니다.
	/// 실제로 메모리를 할당하므로 자주 사용하면 안됩니다.
	/// </summary>
	/// <typeparam name="T">만들 구조체 또는 클래스의 자료형</typeparam>
	/// <param name="offset">버퍼의 위치</param>
	/// <returns>생성된 구조체 또는 클래스 개체</returns>
	public T ReadFrom<T>(int offset) where T : class, new()
	{
		int sz = NetUtils.SizeOf(typeof(T));

		if (0 < sz && IsBoundsFrom(offset, sz))
		{
			var sublength = offset + sz;

			// 부분 배열
			var subspan = myBuffer[offset..sublength];
			T result = NetUtils.Deserialize<T>(in subspan, sz);

			// 뒤쪽 배열
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
	/// 버퍼의 맨 위에서 구조체를 읽어옵니다. 실제로 메모리를 할당하므로 자주 사용하면 안됩니다.
	/// </summary>
	/// <typeparam name="T">만들 구조체 또는 클래스의 자료형</typeparam>
	/// <returns>생성된 구조체 또는 클래스 개체</returns>
	public T Peek<T>() where T : class, new()
	{
		return NetUtils.Deserialize<T>(myBuffer);
	}
	/// <summary>
	/// 지정한 버퍼 위치로부터 구조체를 읽고, 내 버퍼 위치를 조정합니다.
	/// 실제로 메모리를 할당하므로 자주 사용하면 안됩니다.
	/// </summary>
	/// <typeparam name="T">만들 구조체 또는 클래스의 자료형</typeparam>
	/// <param name="offset">버퍼의 위치</param>
	/// <param name="result">결과값을 저장할 변수</param>
	/// <returns>생성된 구조체 또는 클래스 개체</returns>
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
			Debug.LogError("메모리 접근 위반성 오류 발생!\n" + e);
			return false;
		}
		catch (IndexOutOfRangeException e)
		{
			Debug.LogError("배열 참조 오류 발생!\n" + e);
			return false;
		}

		return true;
	}
	/// <summary>
	/// 지정한 버퍼 위치로부터 구조체를 읽고, 내 버퍼 위치를 조정합니다.
	/// 실제로 메모리를 할당하므로 자주 사용하면 안됩니다.
	/// </summary>
	/// <typeparam name="T">만들 구조체 또는 클래스의 자료형</typeparam>
	/// <param name="result">결과값을 저장할 변수</param>
	/// <returns>생성된 구조체 또는 클래스 개체</returns>
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
				Debug.LogError("메모리 접근 위반성 오류 발생!\n" + e);
				return false;
			}
			catch (IndexOutOfRangeException e)
			{
				Debug.LogError("배열 참조 오류 발생!\n" + e);
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
