using UnityEngine;
using System.Collections;
using System.IO; // Used for writing PNG textures to disk
using UnityEngine.Rendering;
using System.Collections.Generic;
using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Serialization;

public class CubemapToFisheye : MonoBehaviour
{

    enum CameraName { Front = 0, Right = 1, Rear = 2, Left = 3 };   

    // Cubemap resolution - should be power of 2: 64, 128, 256 etc.
    public int resolution;
    public string destinationFolderPath;
    public string LUTFolderPath;
    public int cameraNumber;
    public bool calibrationMode;

    //compute shader
    ComputeShader shader;
    int kernelIndex;   
    ComputeBuffer mapBuffer;
    int rowsForShader;
    int columnsForShader;
    List<RenderTexture> fisheyeTextures;

    int counter = 0;
    int fpsCounter = 0;

    //cubemap extractation  
    GameObject cubemapCamera;
    readonly Quaternion[] rotations = { Quaternion.Euler(0, 90, 0), Quaternion.Euler(0, -90, 0), Quaternion.Euler(-90, 0, 0), Quaternion.Euler(90, 0, 0), Quaternion.Euler(0, 0, 0), Quaternion.Euler(0, 180, 0), };
    readonly string[] facesString = {"Right", "Left", "Top", "Bottom", "Front", "Back" };

    #region Cubemap functions

    private byte[] binaryFileToBinaryArray(string filePath, ref int stride, ref int count)
    {
        byte[] fileBytes = File.ReadAllBytes(filePath);

        int rows = BitConverter.ToInt32(fileBytes.Skip(0).Take(4).ToArray(), 0);
        int columns = BitConverter.ToInt32(fileBytes.Skip(4).Take(4).ToArray(), 0);
        int type = BitConverter.ToInt32(fileBytes.Skip(8).Take(4).ToArray(), 0);
        int channels = BitConverter.ToInt32(fileBytes.Skip(12).Take(4).ToArray(), 0);

        rowsForShader = rows;
        columnsForShader = columns;

        stride = 4;
        count = (fileBytes.Length - 16) / stride;

        return fileBytes.Skip(16).Take(fileBytes.Length - 16).ToArray();
    }


    private ComputeBuffer binaryFileToComputeBuffer(int index)
    {
        byte[] buffer = new byte[1];
        int count = 0;
        int stride = 0;

        if (0 == index || 1 ==index ) //maps
        {
            List<byte[]> bufferArray = new List<byte[]>();

            if (0 == index) //xMap
            {
                for (int i = 0; i < 5; ++i)
                {
                    //  string mapXName = LUTFolderPath + "/lut_type_4_cam_" + cameraNumber.ToString() + "_mapX" + i.ToString() + ".bin";
                    string mapXName = LUTFolderPath + "/lut_type_4_cam_0_mapX" + i.ToString() + ".bin";
                    bufferArray.Add(binaryFileToBinaryArray(mapXName, ref stride, ref count));
                }
            }
            else if (1 == index) //yMap
            {
                for (int i = 0; i < 5; ++i)
                {
                    //string mapYName = LUTFolderPath + "/lut_type_4_cam_" + cameraNumber.ToString() + "_mapY" + i.ToString() + ".bin";
                    string mapYName = LUTFolderPath + "/lut_type_4_cam_0_mapY" + i.ToString() + ".bin";

                    bufferArray.Add(binaryFileToBinaryArray(mapYName, ref stride, ref count));
                }
            }
            buffer = new byte[bufferArray[0].Length * 5];
            for (int i = 0; i < 5; ++i)
            {
                System.Buffer.BlockCopy(bufferArray[i], 0, buffer, i * bufferArray[i].Length, bufferArray[i].Length);
            }
            count *= 6;
        }        
        else if(2 == index) //lut
        {
          //  string lutName = LUTFolderPath + "/lut_type_4_cam_" + cameraNumber.ToString() + "LUT.bin";
            string lutName = LUTFolderPath + "/lut_type_4_cam_0LUT.bin";

            buffer = binaryFileToBinaryArray(lutName, ref stride, ref count);          
        }
        else if (3 == index) //GPUmap
        {
            //  string lutName = LUTFolderPath + "/lut_type_4_cam_" + cameraNumber.ToString() + "LUT.bin";
            string mapName = LUTFolderPath + "/GPUmap.bin";

            buffer = binaryFileToBinaryArray(mapName, ref stride, ref count);
        }

        ComputeBuffer cb = new ComputeBuffer(count, stride);    
        cb.SetData(buffer);

        return cb;
    }
         
    // Save a Texture2D as a PNG file
    // http://answers.unity3d.com/questions/245600/saving-a-png-image-to-hdd-in-standalone-build.html
    private void SaveTextureToFile(Texture2D texture, string fileName) {
        byte[] bytes = texture.EncodeToPNG();
        FileStream file = File.Open(fileName, FileMode.Create);
        BinaryWriter binary = new BinaryWriter(file);
        binary.Write(bytes);
        file.Close();
    }

    #endregion


    void OnPreRender()
    {
        // before rendering, setup our RenderTexture
        if (0 != fisheyeTextures.Count)
        {
            GetComponent<Camera>().targetTexture = fisheyeTextures[fisheyeTextures.Count - 1];
        }
    }

    void OnRenderImage(RenderTexture src, RenderTexture dest)
    {
        if (0 != fisheyeTextures.Count)
        {
            Graphics.Blit(fisheyeTextures[fisheyeTextures.Count - 1], null as RenderTexture);
        }
    }

    void OnPostRender()
    {
        Camera screenCam = GetComponent<Camera>();

        if (0 != fisheyeTextures.Count)
        {
            screenCam.targetTexture = null;
            RenderTexture.active = null;           
        }
    }

    // Use this for initialization
    void Start ()
    {
      
        fisheyeTextures = new List<RenderTexture>();     

        cubemapCamera = new GameObject("CubemapCamera", typeof(Camera));
    
        if (0 == destinationFolderPath.Length)
        {
            destinationFolderPath = Application.persistentDataPath;
        }
        if (!Directory.Exists(destinationFolderPath))
        {
            DirectoryInfo di = Directory.CreateDirectory(destinationFolderPath);
        }
        destinationFolderPath += "/" + ((CameraName)cameraNumber).ToString("g");
        if (!Directory.Exists(destinationFolderPath))
        {
            DirectoryInfo di = Directory.CreateDirectory(destinationFolderPath);
        }
       
        mapBuffer = binaryFileToComputeBuffer(3);

        shader = Instantiate(Resources.Load("CubemapToFisheye")) as ComputeShader;
        kernelIndex = shader.FindKernel("CSMain");
       
        shader.SetBuffer(kernelIndex, Shader.PropertyToID("mapBuffer"), mapBuffer);
        shader.SetInt(Shader.PropertyToID("columns"), columnsForShader);
        shader.SetInt(Shader.PropertyToID("rows"), rowsForShader);
        shader.SetInt(Shader.PropertyToID("cubemapSize"), resolution); //lut size

        Camera screenCam = GetComponent<Camera>();        
    }

    // This is the coroutine that creates the cubemap images
    IEnumerator CreateCubeMap()
    {
        // Place the camera on this object
        cubemapCamera.transform.position = transform.position;
        // Initialise the rotation - this will be changed for each texture grab
       cubemapCamera.transform.rotation = transform.rotation;

        Camera camera = cubemapCamera.GetComponent<Camera>();
        camera.transform.position = transform.position;
        camera.fieldOfView = 90.0f;
        camera.depth = -1;
        camera.nearClipPlane = 0.1f;
        camera.enabled = false;  

        yield return new WaitForEndOfFrame();
      
        RenderTexture fisheyeOutput = new RenderTexture(columnsForShader, rowsForShader, 24);
        fisheyeOutput.enableRandomWrite = true;
        fisheyeOutput.Create();
        shader.SetTexture(kernelIndex, Shader.PropertyToID("Result"), fisheyeOutput);

        for (int i = 0; i < 5; i++)
        {
            camera.transform.rotation = transform.rotation;
            camera.transform.rotation *= rotations[i];
            RenderTexture renderTexture = new RenderTexture(resolution, resolution, 24);
            renderTexture.filterMode = FilterMode.Bilinear;
            camera.targetTexture = renderTexture;
            camera.Render();        
            shader.SetTexture(kernelIndex, Shader.PropertyToID(facesString[i]), renderTexture);
        }
        shader.Dispatch(kernelIndex, columnsForShader / 8, rowsForShader / 8, 1);
        fisheyeTextures.Add(fisheyeOutput);  
    }

    
    // Update is called once per frame
    void Update ()
    {
        if (calibrationMode)
        {
            if (Input.GetKeyDown("enter") || Input.GetKeyDown("return"))
            {
                StartCoroutine(CreateCubeMap());
            }
        }
        else
        {
            ++fpsCounter;
            if (0 == fpsCounter % 5)
            {
                StartCoroutine(CreateCubeMap());
            }
        }
    }

    private void OnDestroy()
    {      
        mapBuffer.Dispose();
        Texture2D save = new Texture2D(columnsForShader, rowsForShader, TextureFormat.RGB24, false);
        save.wrapMode = TextureWrapMode.Clamp;
        foreach (var rt in fisheyeTextures)
        {           
            RenderTexture.active = rt;
            save.ReadPixels(new Rect(0, 0, rt.width, rt.height), 0, 0);
            string filePath = destinationFolderPath + "/" + counter.ToString()+ ".png";
            SaveTextureToFile(save, filePath);
            ++counter;
        }
    }
}