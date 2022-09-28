using System;
using System.Collections;
using System.Collections.Generic;

using UnityEngine;

[Serializable]
public class NetUser
{
	[Header("속성")]
	[Tooltip("별명")]
	public string myName;
	public string myEmail;

	public bool isLoggedIn = false;
}
