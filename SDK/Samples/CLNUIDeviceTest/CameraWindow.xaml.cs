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
using System.Diagnostics;

namespace CLNUIDeviceTest
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class CameraWindow : Window
    {
        private IntPtr motor = IntPtr.Zero;
        private DispatcherTimer accelerometerTimer;

        private IntPtr camera = IntPtr.Zero;
        private NUIImage colorImage;
        private NUIImage depthImage;

        private Thread captureThread;
        private bool running;

        public CameraWindow(string devSerial)
        {
            InitializeComponent();
            Closing += new System.ComponentModel.CancelEventHandler(OnClosing);

            xp.Minimum = -981;
            xp.Maximum = 981;
            yp.Minimum = -981;
            yp.Maximum = 981;
            zp.Minimum = -981;
            zp.Maximum = 981;

            try
            {
                motor = CLNUIDevice.CreateMotor(devSerial);
                camera = CLNUIDevice.CreateCamera(devSerial);
            }
            catch (System.Exception ex)
            {
                MessageBox.Show(ex.Message);
                Environment.Exit(0);
            }

            CLNUIDevice.SetMotorPosition(motor, 0);

            serial.Content = string.Format("Serial Number: {0}", devSerial);
            accelerometerTimer = new DispatcherTimer(TimeSpan.FromMilliseconds(100), DispatcherPriority.Normal, (EventHandler)delegate(object sender, EventArgs e) 
            {
                short _x = 0, _y = 0, _z = 0;
                CLNUIDevice.GetMotorAccelerometer(motor, ref _x, ref _y, ref _z);
                x.Content = _x.ToString();
                xp.Value = _x;
                y.Content = _y.ToString();
                yp.Value = _y;
                z.Content = _z.ToString();
                zp.Value = _z;
            }, Dispatcher);
            accelerometerTimer.Start();

            led.SelectionChanged += new SelectionChangedEventHandler(led_SelectionChanged);
            led.SelectedIndex = 0;

            colorImage = new NUIImage(640, 480);
            color.Source = colorImage.BitmapSource;

            depthImage = new NUIImage(640, 480);
            depth.Source = depthImage.BitmapSource;

            running = true;
            captureThread = new Thread(delegate() 
            {
//                 Trace.WriteLine(string.Format("Camera {0:X}", camera.ToInt32()));
                if (CLNUIDevice.StartCamera(camera))
                {
                    while (running)
                    {
                        CLNUIDevice.GetCameraColorFrameRGB32(camera, colorImage.ImageData, 500);
                        CLNUIDevice.GetCameraDepthFrameRGB32(camera, depthImage.ImageData, 0);
                        Dispatcher.BeginInvoke(DispatcherPriority.Normal, (Action)delegate()
                        {
                            colorImage.Invalidate();
                            depthImage.Invalidate();
                        });
                    }
                    CLNUIDevice.StopCamera(camera);
                }
            });
            captureThread.IsBackground = true;
            captureThread.Start();
        }

        void led_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            CLNUIDevice.SetMotorLED(motor, (byte)led.SelectedIndex);
        }

        void OnClosing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            accelerometerTimer.Stop();
            CLNUIDevice.SetMotorLED(motor, 0);
            if (motor != IntPtr.Zero)   CLNUIDevice.DestroyMotor(motor);
            if (captureThread != null)
            {
                running = false;
                captureThread.Join(100);
            }
            if (camera != IntPtr.Zero)  CLNUIDevice.DestroyCamera(camera);
        }

        private void up_Click(object sender, RoutedEventArgs e)
        {
            CLNUIDevice.SetMotorPosition(motor, 8000);
        }

        private void down_Click(object sender, RoutedEventArgs e)
        {
            CLNUIDevice.SetMotorPosition(motor, -8000);
        }
    }
}
