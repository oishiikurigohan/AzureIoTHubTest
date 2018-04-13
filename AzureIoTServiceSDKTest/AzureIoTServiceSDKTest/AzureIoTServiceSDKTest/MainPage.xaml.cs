using System;
using Xamarin.Forms;
using System.Threading.Tasks;
using Microsoft.Azure.Devices;

namespace AzureIoTServiceSDKTest
{
	public partial class MainPage : ContentPage
	{
        static string connectionString = "HostName=MyIoTHub20180408.azure-devices.net;SharedAccessKeyName=iothubowner;SharedAccessKey=fXgnUTFZ09VBW6wtoreY3FN6ovKCuh9V6a3KleVvWQc=";
        static string deviceId = "RaspberryPi2";
        static string ledOnMethodName = "LED_ON";
        static string ledOffMethodName = "LED_OFF";
        static ServiceClient serviceClient;       

        public MainPage()
		{
			InitializeComponent();
            serviceClient = ServiceClient.CreateFromConnectionString(connectionString);
        }

        void OnButton_Clicked(object sender, System.EventArgs e)
        {
            InvokeMethod(ledOnMethodName).Wait();
            LEDImage.Source = "on.png";
        }

        void OffButton_Clicked(object sender, System.EventArgs e)
        {
            InvokeMethod(ledOffMethodName).Wait();
            LEDImage.Source = "off.png";
        }

        private static async Task InvokeMethod(string methodName)
        {
            var methodInvocation = new CloudToDeviceMethod(methodName) { ResponseTimeout = TimeSpan.FromSeconds(10) };
            methodInvocation.SetPayloadJson("{\"Message\":\"Invoke " + methodName + "\"}");
            var response = await serviceClient.InvokeDeviceMethodAsync(deviceId, methodInvocation).ConfigureAwait(false);
        }
    }
}