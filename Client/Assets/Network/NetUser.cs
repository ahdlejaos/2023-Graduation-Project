using System;
using System.Collections;
using System.Collections.Generic;

using UnityEngine;

[Serializable]
public class NetUser
{
	[Header("�Ӽ�")]
	[Tooltip("����")]
	public string myName;
	public string myEmail;

	public bool isLoggedIn = false;
}
