using System;

using System.Windows.Forms;
using System.Net;
using System.Threading.Tasks;
using System.IO;
using System.Drawing.Imaging;

using Emgu.CV;
using System.Runtime.InteropServices;
using System.Threading;

namespace WFApp1
{
        public partial class Form1 : Form
        {

                private HttpListener _httpListener = null;
                private VideoCapture _videoCapture = null;
                private Task _httlListenerTask = null;
                private string _baseDir = null;
                private object _videoCaptureLock = new object();

                
                [DllImport("wfdll1.dll", CharSet = CharSet.Unicode)]
                private static extern int BiometricCapture(String sequence, IntPtr hWND);

                delegate void AddLogCallback(string message);
                
                public Form1()
                {
                        InitializeComponent();
                        
                        _baseDir = Application.StartupPath + "\\temppreviewpictures";
                        if (!Directory.Exists(_baseDir))
                                Directory.CreateDirectory(_baseDir);

                        _videoCapture = new VideoCapture();

                        InitializeHttpListener();
            
                }

                private static void CloseResponse(HttpListenerResponse response, int statuscode)
                {
                        response.StatusCode = statuscode;
                        response.Close();
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

                        _httlListenerTask = new Task(() => {
                                
                                while (_httpListener.IsListening)
                                {

                                        HttpListenerContext context = _httpListener.GetContext();
                                        HttpListenerRequest request = context.Request;
                                        HttpListenerResponse response = context.Response;
                                        int statusCode = 500;

                                        try
                                        {
                                                if (!request.IsLocal)
                                                        continue;

                                                byte[] buffer = null;
                                                String sequence = _baseDir + "\\${sequence}_" +
                                                                request.QueryString.Get(0) + ".bmp";

                                                AddLog("processing a request " + request.Url.PathAndQuery);
        
                                                if (request.Url.PathAndQuery.Contains("/fingerprint/"))
                                                {
                                                        sequence = sequence.Replace("${sequence}", "biofingerprint");
                                                        int hr = BiometricCapture(sequence, this.Handle);
                                                        if (hr != 0L)
                                                        {
                                                                AddLog("an error (" + hr + ") is occured while taking fingerprint");
                                                                continue;
                                                        }

                                                        using (FileStream fs = File.OpenRead(sequence))
                                                        {
                                                                buffer = new byte[fs.Length];
                                                                fs.Read(buffer, 0, buffer.Length);
                                                                statusCode = 200;
                                                                AddLog("processed: " + request.Url.PathAndQuery);
                                                        }

                                                }
                                                else if (request.Url.PathAndQuery.Contains("/camera/preview/"))
                                                {
                                                        if (!_videoCapture.IsOpened)
                                                                continue;
                                                        using (Mat mat = _videoCapture.QueryFrame())
                                                        {
                                                                if (mat == null)
                                                                        continue;
                                                                if (mat.IsEmpty)
                                                                        continue;
                                                                if (mat.IsContinuous)
                                                                        Thread.Sleep(500);

                                                                AddLog("captured: " + request.Url.PathAndQuery);

                                                                using (MemoryStream ms = new MemoryStream())
                                                                {
                                                                        mat.Bitmap.Save(ms, ImageFormat.Bmp);
                                                                        buffer = ms.ToArray();
                                                                        sequence = sequence.Replace("${sequence}", "biophoto");
                                                                        using (FileStream fs = File.OpenWrite(sequence))
                                                                        {
                                                                                fs.Write(buffer, 0, buffer.Length);
                                                                                statusCode = 200;
                                                                                AddLog("processed: " + request.Url.PathAndQuery);
                                                                        }
                                                                }
                                                        }
                                                        

                                                }
                                                else if (request.Url.PathAndQuery.Contains("/camera/"))
                                                {
                                                        if (!_videoCapture.IsOpened)
                                                                continue;

                                                        using (Mat mat = _videoCapture.QueryFrame())
                                                        {
                                                                if (mat == null)
                                                                        continue;
                                                                AddLog("captured: " + request.Url.PathAndQuery);

                                                                using (MemoryStream ms = new MemoryStream())
                                                                {
                                                                        mat.Bitmap.Save(ms, ImageFormat.Bmp);
                                                                        buffer = ms.ToArray();
                                                                        statusCode = 200;
                                                                        AddLog("processed: " + request.Url.PathAndQuery);
                                                                }
                                                        }
                                                                
                                                }

                                                if (statusCode != 200)
                                                        continue;

                                                response.ContentType = "application/image";
                                                response.OutputStream.Write(buffer, 0, buffer.Length);
                                                
                                                AddLog("finished: " + request.Url.PathAndQuery);
                                                
                                        }
                                        finally
                                        {
                                                response.StatusCode = statusCode;
                                                response.Close();

                                        }

                                }
                        });

                        _httlListenerTask.Start();

                        logListBox.Items.Add("HttpListener is started.");

                }

                
        
        }
}
