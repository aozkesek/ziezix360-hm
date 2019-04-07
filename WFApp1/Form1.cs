using System;

using System.Windows.Forms;
using System.Net;
using System.Threading.Tasks;
using System.IO;
using System.Drawing.Imaging;

using Emgu.CV;
using Emgu.CV.CvEnum;

namespace WFApp1
{
    public partial class Form1 : Form
    {

        private VideoCapture _videoCapture;
        private HttpListener _httpListener;
        private Task _httlListenerTask;

        delegate void AddLogCallback(string message);
        
        public Form1()
        {
            InitializeComponent();
            
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

        private void InitializeHttpListener()
        {
            logListBox.Items.Add("checking if this system is supporting HttpListener...");
            if (!HttpListener.IsSupported)
                return;

            _httpListener = new HttpListener();

            _httpListener.Prefixes.Add("http://localhost:8081/capture/");
            _httpListener.Prefixes.Add("http://localhost:8081/capture/preview/");

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

                    bool isCapture = request.Url.PathAndQuery.Contains("/capture/preview/") ? false : true;

                    HttpListenerResponse response = context.Response;

                    string temp = Application.StartupPath + "\\temppreviewpictures";
                    if (!Directory.Exists(temp))
                        Directory.CreateDirectory(temp);

                    Mat mat = _videoCapture.QueryFrame();
                    if (mat == null)
                        continue;

                    byte[] buffer;
                    using (MemoryStream ms = new MemoryStream())
                    {
                        mat.Bitmap.Save(ms, ImageFormat.Bmp);
                        buffer = ms.ToArray();
                    }

                    if (!isCapture)
                    {
                        FileStream fs = File.OpenWrite(temp + "\\previewPicture_" + request.QueryString.Get(0) + ".bmp");
                        fs.Write(buffer, 0, buffer.Length);
                        fs.Close();
                        fs.Dispose();
                    }

                    response.ContentType = "application/image";
                    try
                    {
                        await response.OutputStream.WriteAsync(buffer, 0, buffer.Length);
                        response.StatusCode = 200;
                    }
                    catch(Exception e)
                    {
                        AddLog("An error is occured: " + e.Message);
                        response.StatusCode = 500;
                    }
                    finally
                    {
                        response.Close();
                    }
                    
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
