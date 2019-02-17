using UnityEngine;
using System.Collections;

public class CheckerBoardController : MonoBehaviour
{
   // private vector3 p
    private float x;
    private float y;
    private float z;
    private float pitch;
    private float roll;
    private float yaw;
    private Vector3 rotateValue;
    public float ControlSensitivityRotation = 0.5f;
    public float ControlSensitivityPosition = 0.1f;


    void Update()
    {
        x = 0;
        y = 0;
        z = 0;
        pitch = 0;
        roll = 0;
        yaw = 0;

        if (Input.GetKey("right"))
        {
            x = ControlSensitivityPosition;
        }
        else if (Input.GetKey("left"))
        {
            x = -ControlSensitivityPosition;
        }
        if (Input.GetKey("up"))
        {
            y = ControlSensitivityPosition;
        }
        else if (Input.GetKey("down"))
        {
            y = -ControlSensitivityPosition;
        }
        if (Input.GetKey("[+]"))
        {
            z = ControlSensitivityPosition;
        }
        else if (Input.GetKey("[-]"))
        {
            z = -ControlSensitivityPosition;
        }
        if (Input.GetKey("[8]"))
        {
            pitch = ControlSensitivityRotation;
        }
        else if (Input.GetKey("[2]"))
        {
            pitch = -ControlSensitivityRotation;
        }
        if (Input.GetKey("[4]"))
        {
            yaw = ControlSensitivityRotation;
        }
        else if (Input.GetKey("[6]"))
        {
            yaw = -ControlSensitivityRotation;
        }
        if (Input.GetKey("[7]"))
        {
            roll = ControlSensitivityRotation;
        }
        else if (Input.GetKey("[9]"))
        {
            roll = -ControlSensitivityRotation;
        }

        transform.position += new Vector3(x, y, z);
        //transform.eulerAngles += new Vector3(-pitch, yaw, roll);
        transform.rotation *= Quaternion.Euler(-pitch, yaw, roll);
    }
}