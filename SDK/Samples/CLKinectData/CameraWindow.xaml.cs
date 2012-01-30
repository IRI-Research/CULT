//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This library is part of CL NUI SDK
// It allows the use of Microsoft Kinect cameras in your own applications
//
// For updates and file downloads go to: http://codelaboratories.com/get/kinect
//
// Copyright 2010 (c) Code Laboratories, Inc.  All rights reserved.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using System.Runtime.InteropServices;
using System.Windows.Interop;
using System.Threading;
using System.ComponentModel;

namespace CLNUIDeviceTest
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class CameraWindow : Window, INotifyPropertyChanged
    {
        #region ViewModelProperty: TheCharacter
        private string _theCharacter;
        public string TheCharacter
        {
            get
            {
                return _theCharacter;
            }

            set
            {
                _theCharacter = value;
                OnPropertyChanged("TheCharacter");
            }
        }
        #endregion

        #region ViewModelProperty: FromLeft
        private int _fromLeft;
        public int FromLeft
        {
            get
            {
                return _fromLeft;
            }

            set
            {
                _fromLeft = value;
                OnPropertyChanged("FromLeft");
            }
        }
        #endregion

        #region ViewModelProperty: FromTop
        private int _fromTop;
        public int FromTop
        {
            get
            {
                return _fromTop;
            }

            set
            {
                _fromTop = value;
                OnPropertyChanged("FromTop");
            }
        }
        #endregion

        #region ViewModelProperty: TheFontSize
        private int _theFontSize;
        public int TheFontSize
        {
            get
            {
                return _theFontSize;
            }

            set
            {
                _theFontSize = value;
                OnPropertyChanged("TheFontSize");
            }
        }
        #endregion

        private int stepSize = 5;
        private IntPtr motor = IntPtr.Zero;
        private DispatcherTimer accelerometerTimer;
        int _xCal = 0;
        int _yCal = 0;
        int _zCal = 0;
        bool campaused = false;
        bool accpaused = false;
        bool errquit = false;
        private IntPtr camera = IntPtr.Zero;
        private NUIImage colorImage;
        private Thread captureThread;
        private bool running; 
        private string devSerial;

        public CameraWindow(string devSerial)
        {
            this.devSerial = devSerial;
            InitializeComponent();
            Closing += new System.ComponentModel.CancelEventHandler(MainWindow_Closing);
            try
            {
                motor = CLNUIDevice.CreateMotor(devSerial);
                camera = CLNUIDevice.CreateCamera(devSerial);
                labelSERIAL.Content = string.Format("Serial Number: {0}", devSerial);
            }
            catch
            {
                MessageBox.Show("Is your device connected! Drivers installed?", "Kinect Not Found!");
                errquit = true;
                this.Close();
            }
            DataContext = this;
            TheCharacter = "█";
            FromLeft = 526;
            FromTop = 136;
            TheFontSize = 5;
            accelerometerTimer = new DispatcherTimer(TimeSpan.FromMilliseconds(100), DispatcherPriority.Normal, (EventHandler)delegate(object sender, EventArgs e)
            {
                short _x = 0, _y = 0, _z = 0;
                CLNUIDevice.GetMotorAccelerometer(motor, ref _x, ref _y, ref _z);
                double gx = 0;
                double gy = 0;
                _x = (short)(_x - _xCal);
                _y = (short)(_y - _yCal);
                _z = (short)(_z - _zCal);
                if (comboXAXIS.SelectedIndex == 0)
                {
                    gx = _x + 1000;
                }
                else if (comboXAXIS.SelectedIndex == 1)
                {
                    gx = _y + 1000;
                }
                else if (comboXAXIS.SelectedIndex == 2)
                {
                    gx = _z + 1000;
                }
                if (comboYAXIS.SelectedIndex == 0)
                {
                    gy = _x + 1000;
                }
                else if (comboYAXIS.SelectedIndex == 1)
                {
                    gy = _y + 1000;
                }
                else if (comboYAXIS.SelectedIndex == 2)
                {
                    gy = _z + 1000;
                }
                FromLeft = 389 + (int)(gx / 9.661835748792271);
                FromTop = 34 + (int)(gy / 9.661835748792271);
                labelX.Content = "X: " + (_x).ToString();
                progressX.Value = _x;
                labelY.Content = "Y: " + (_y).ToString();
                progressY.Value = _y;
                labelZ.Content = "Z: " + (_z).ToString();
                progressZ.Value = _z;
            }, Dispatcher);
            accelerometerTimer.Start();

            comboLED.SelectionChanged += new SelectionChangedEventHandler(comboLED_SelectionChanged);
            comboLED.SelectedIndex = 0;
            comboFEED.SelectionChanged += new SelectionChangedEventHandler(comboBox1_SelectionChanged);
            comboFEED.SelectedIndex = 0;
            comboXAXIS.SelectedIndex = 0;
            comboYAXIS.SelectedIndex = 1;
            comboTheSize.SelectionChanged += new SelectionChangedEventHandler(comboBox1_SelectionChanged_1);
            comboTheSize.SelectedIndex = 5;
            sliderTILT.Maximum = 8000;
            sliderTILT.Minimum = -8000;
            sliderTILT.Value = 0;
            CLNUIDevice.SetMotorPosition(motor, 0);
        }

        void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (errquit == false)
            {
                if (campaused == true) captureThread.Resume();
                if (accpaused == false) accelerometerTimer.Stop();
                CLNUIDevice.SetMotorLED(motor, 0);
                if (motor != IntPtr.Zero)
                    CLNUIDevice.DestroyMotor(motor);
                running = false;
                captureThread.Join();
                if (camera != IntPtr.Zero)
                {
                    CLNUIDevice.StopCamera(camera);
                    CLNUIDevice.DestroyCamera(camera);
                }
            }
        }

        void comboBox1_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (running == true)
            {
                running = false;
                captureThread.Join();
                if (camera != IntPtr.Zero)
                {
                    CLNUIDevice.StopCamera(camera);
                    CLNUIDevice.DestroyCamera(camera);
                }
                camera = CLNUIDevice.CreateCamera(devSerial);
            }
            if (comboFEED.SelectedIndex == 0)
            {
                colorImage = new NUIImage(640, 480);
                feed.Source = colorImage.BitmapSource;
                running = true;
                captureThread = new Thread(delegate()
                {
                    CLNUIDevice.StartCamera(camera);
                    while (running)
                    {
                        CLNUIDevice.GetCameraColorFrameRGB32(camera, colorImage.ImageData, 500);
                        Dispatcher.BeginInvoke(DispatcherPriority.Normal, (Action)delegate()
                        {
                            colorImage.Invalidate();
                        });
                    }
                    CLNUIDevice.StopCamera(camera);
                });
                captureThread.IsBackground = true;
                captureThread.Start();
            }
            else
            {
                colorImage = new NUIImage(640, 480);
                feed.Source = colorImage.BitmapSource;
                running = true;
                captureThread = new Thread(delegate()
                {
                    CLNUIDevice.StartCamera(camera);
                    while (running)
                    {
                        CLNUIDevice.GetCameraDepthFrameRGB32(camera, colorImage.ImageData, 500);
                        Dispatcher.BeginInvoke(DispatcherPriority.Normal, (Action)delegate()
                        {
                            colorImage.Invalidate();
                        });
                    }
                    CLNUIDevice.StopCamera(camera);
                });
                captureThread.IsBackground = true;
                captureThread.Start();
            }
        }

        private void sliderTILT_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            int val = (int)sliderTILT.Value;
            CLNUIDevice.SetMotorPosition(motor, (short)val);
            textBoxTILT.Text = val.ToString();
        }

        #region INotifiedProperty Block
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged(string propertyName)
        {
            PropertyChangedEventHandler handler = PropertyChanged;

            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(propertyName));
            }
        }
        #endregion

        private void comboLED_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            CLNUIDevice.SetMotorLED(motor, (byte)comboLED.SelectedIndex);
        }

        private void button1_Click_1(object sender, RoutedEventArgs e)
        {
            accelerometerTimer.Stop();
        }

        private void comboBox1_SelectionChanged_1(object sender, SelectionChangedEventArgs e)
        {
            TheFontSize = (int)comboTheSize.SelectedIndex;
        }

        private void button1_Click_2(object sender, RoutedEventArgs e)
        {
            _xCal = (int)progressX.Value + _xCal;
            _yCal = (int)progressY.Value + _yCal;
            _zCal = (int)progressZ.Value + _zCal;
        }

        private void button2_Click(object sender, RoutedEventArgs e)
        {
            _xCal = 0;
            _yCal = 0;
            _zCal = 0;
        }

        private void button3_Click(object sender, RoutedEventArgs e)
        {
            if (campaused == false)
            {
                captureThread.Suspend();
                campaused = true;
                button3.Content = "Resume";
            }
            else if (campaused == true)
            {
                captureThread.Resume();
                campaused = false;
                button3.Content = "Camera";
            }
        }

        private void button4_Click(object sender, RoutedEventArgs e)
        {
            if (accpaused == false)
            {
                accelerometerTimer.Stop();
                accpaused = true;
                button4.Content = "Resume";
            }
            else if (accpaused == true)
            {
                accelerometerTimer.Start();
                accpaused = false;
                button4.Content = "Accelerometer";
            }
        }
    }
}
