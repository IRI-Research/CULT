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
using System.Windows.Shapes;

namespace CLNUIDeviceTest
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        CameraWindow[] cameraWindows = new CameraWindow[16];
        int _devCount;
        public MainWindow()
        {
            InitializeComponent();
            Closing += new System.ComponentModel.CancelEventHandler(MainWindow_Closing);
            try
            {
                _devCount = CLNUIDevice.GetDeviceCount();
                if (_devCount == 0)
                {
                    Environment.Exit(0);
                }
                for (int i = 0; i < _devCount; i++)
                {
                    string devSerial = CLNUIDevice.GetDeviceSerial(i);
                    cameras.Items.Add(string.Format("Kinect Serial: {0}", devSerial));
                    cameraWindows[i] = new CameraWindow(devSerial);
                    cameraWindows[i].Show();
                }
            }
            catch { }
        }

        void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            for (int i = 0; i < _devCount; i++)
            {
                cameraWindows[i].Close();
            }
        }
    }
}
