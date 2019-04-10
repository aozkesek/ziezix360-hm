using System;

using System.Windows.Forms;
using System.Net;
using System.Threading.Tasks;
using System.IO;
using System.Drawing.Imaging;

using Emgu.CV;
using Emgu.CV.CvEnum;
using System.Runtime.InteropServices;

namespace WFApp1
{
        public partial class Form1 : Form
        {

                private VideoCapture _videoCapture;
                private HttpListener _httpListener;
                private Task _httlListenerTask;
                private string _baseDir;

                [DllImport("wfdll1.dll", CharSet = CharSet.Unicode)]
                static extern int BiometricCapture(String sequence, IntPtr hWND);

                delegate void AddLogCallback(string message);
        
                public Form1()
                {
                        InitializeComponent();
                        
                        _baseDir = Application.StartupPath + "\\temppreviewpictures";
                        if (!Directory.Exists(_baseDir))
                                Directory.CreateDirectory(_baseDir);

                        InitializeVideoCapture();

                        InitializeHttpListener();
            
                }

                // to add any log from cross thread  
                private void AddLog(string message)
                {
                        if (logListBox.InvokeRequired)
                                this.Invoke(new AddLogCallback(AddLog), new object[] { message });
                        else
                        {
                                logListBox.Items.Add(message);
                                logListBox.SelectedIndex = logListBox.Items.Count - 1;
                        }
                
                }

                private async void CaptureCameraFrames(HttpListenerContext context, 
                        bool keepFrame = false)
                {
                        HttpListenerResponse response = context.Response;
                        HttpListenerRequest request = context.Request;

                        Mat mat = _videoCapture.QueryFrame();
                        if (mat == null)
                                return;

                        byte[] buffer;
                        using (MemoryStream ms = new MemoryStream())
                        {
                                mat.Bitmap.Save(ms, ImageFormat.Bmp);
                                buffer = ms.ToArray();
                        }

                        if (keepFrame)
                        {
                                string sequence = _baseDir + "\\previewPicture_" +
                                        request.QueryString.Get(0) + ".bmp";
                                using (FileStream fs = File.OpenWrite(sequence))
                                {
                                        fs.Write(buffer, 0, buffer.Length);
                                }
                        }

                        response.ContentType = "application/image";
                        try
                        {
                                await response.OutputStream.WriteAsync(buffer, 0, buffer.Length);
                                response.StatusCode = 200;
                        }
                        catch (Exception e)
                        {
                                AddLog("An error is occured: " + e.Message);
                                response.StatusCode = 500;
                        }
                        finally
                        {
                                response.Close();
                        }

                }

                private async void CaptureFingerprintFrame(HttpListenerContext context)
                {
                        HttpListenerResponse response = context.Response;
                        HttpListenerRequest request = context.Request;

                        String sequence = _baseDir + "\\previewPicture_" + 
                                request.QueryString.Get(0) + ".bmp";


                        this.BringToFront();
                        this.Activate();
                        this.logListBox.Focus();
                        
                        int hr = BiometricCapture(sequence, this.Handle);
                        if (hr != 0L)
                        {
                                AddLog("an error ("+hr+") is occured while taking fingerprint");
                                response.StatusCode = 500;
                                response.Close();
                                return;
                        }

                        AddLog("Fingerprint sample is captured.");
                        byte[] buffer;

                        using (FileStream fs = File.OpenRead(sequence))
                        {
                                buffer = new byte[fs.Length];
                                fs.Read(buffer, 0, buffer.Length);
                        }

                        response.ContentType = "application/image";
                        try
                        {
                                await response.OutputStream.WriteAsync(buffer, 0, buffer.Length);
                                response.StatusCode = 200;
                        }
                        catch (Exception e)
                        {
                                AddLog("An error is occured: " + e.Message);
                                response.StatusCode = 500;
                        }
                        finally
                        {
                                response.Close();
                        }


                }

                private void InitializeHttpListener()
                {
                        logListBox.Items.Add("checking if this system is supporting HttpListener...");
                        if (!HttpListener.IsSupported)
                                return;

                        _httpListener = new HttpListener();

                        _httpListener.Prefixes.Add("http://localhost:8081/capture/camera/");
                        _httpListener.Prefixes.Add("http://localhost:8081/capture/camera/preview/");
                        _httpListener.Prefixes.Add("http://localhost:8081/capture/fingerprint/");

                        logListBox.Items.Add("HttpListener is starting for http://localhost:8081/capture/ ...");

                        _httpListener.Start();

                        // start a task that listener 
                        _httlListenerTask = new Task(async () => {
                                while(_httpListener.IsListening)
                                {
                                        HttpListenerContext context = await _httpListener.GetContextAsync();
                                        HttpListenerRequest request = context.Request;

                                        if (!request.IsLocal)
                                                continue;

                                        AddLog("processing a request " + request.Url.PathAndQuery);

                                        if (request.Url.PathAndQuery.Contains("/fingerprint/"))
                                                CaptureFingerprintFrame(context);
                                        else if (request.Url.PathAndQuery.Contains("/camera/preview/"))
                                                CaptureCameraFrames(context, true);
                                        else if (request.Url.PathAndQuery.Contains("/camera/"))
                                                CaptureCameraFrames(context);

                    
                                }
                        });

                        _httlListenerTask.Start();

                        logListBox.Items.Add("HttpListener is started.");

                }

                private bool InitializeVideoCapture()
                {
                        logListBox.Items.Add("Checking if the system has a capture device...");

                        _videoCapture = new VideoCapture(CaptureType.Any);
                        if (!_videoCapture.IsOpened)
                                return false;

                        logListBox.Items.Add("Video Capture device is ready ...");

                        return true;
                }

        
        }
}
