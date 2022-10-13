using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraMovement : MonoBehaviour
{
    public Transform m_ObjectToFollow;
    public float m_FollowSpeed;
    public float m_Sensitivity;
    public float m_ClampAngle;
    public float m_Smoothness;

    private float m_RotX;
    private float m_RotY;

    public Transform m_Camera;
    public Vector3 m_DirNormalized;
    public Vector3 m_FinalDir;
    public float m_MinDistance;
    public float m_MaxDistance;
    public float m_FinalDistance;
    private void Start()
    {
        m_RotX = transform.localRotation.eulerAngles.x;
        m_RotY = transform.localRotation.eulerAngles.y;

        m_DirNormalized = m_Camera.localPosition.normalized;
        m_FinalDistance = m_Camera.localPosition.magnitude;

        Cursor.lockState = CursorLockMode.Locked;
        Cursor.visible = false;
    }
    private void Update()
    {
        m_RotX += -(Input.GetAxis("Mouse Y")) * m_Sensitivity * Time.deltaTime;
        m_RotY += Input.GetAxis("Mouse X") * m_Sensitivity * Time.deltaTime;

        m_RotX = Mathf.Clamp(m_RotX, -m_ClampAngle, m_ClampAngle);
        Quaternion rot = Quaternion.Euler(m_RotX, m_RotY, 0);
        transform.rotation = rot;

    }
    private void LateUpdate()
    {
        transform.position = Vector3.MoveTowards(transform.position, m_ObjectToFollow.position, m_FollowSpeed * Time.deltaTime);
        m_FinalDir = transform.TransformPoint(m_DirNormalized * m_MaxDistance);

        RaycastHit hit;

        if(Physics.Linecast(transform.position,m_FinalDir,out hit))
        {
            m_FinalDistance = Mathf.Clamp(hit.distance, m_MinDistance, m_MaxDistance);
        }
        else
        {
            m_FinalDistance = m_MaxDistance;
        }
        m_Camera.localPosition = Vector3.Lerp(m_Camera.localPosition, m_DirNormalized * m_FinalDistance, Time.deltaTime * m_Smoothness);
    }
}
