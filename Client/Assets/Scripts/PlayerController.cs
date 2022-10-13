using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerController : MonoBehaviour
{
    private Animator m_Animator;
    [SerializeField]
    private float m_RunSpeed;

    private Rigidbody m_Rigidbody;

    private void Awake()
    {
        m_Animator = GetComponent<Animator>();
        m_Rigidbody = GetComponent<Rigidbody>();
    }
    private void Update()
    {
        Move();
        if (Input.GetKeyDown(KeyCode.I))
        {
            m_Animator.Play("Idle");
        }
        else if (Input.GetKeyDown(KeyCode.R))
        {        
            m_Animator.Play("Run");
        }
    }

    private void Move()
    {
        float _MoveDIrX = Input.GetAxisRaw("Horizontal");
        float _MoveDIrZ = Input.GetAxisRaw("Vertical");
        Vector3 _MoveHorizontal = transform.right * _MoveDIrX;
        Vector3 _MoveVertical = transform.forward * _MoveDIrZ;

        Vector3 _Velocity = (_MoveHorizontal + _MoveVertical).normalized * m_RunSpeed;
        m_Rigidbody.MovePosition(transform.position + _Velocity * Time.deltaTime);

    }
}
