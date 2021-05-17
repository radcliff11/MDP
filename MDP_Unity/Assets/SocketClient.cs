using System;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using UnityEngine;
using static Algo;

public class SocketClient {
    #region private members 	
    public static TcpClient socketConnection;
    public static Tuple<string, int> ADDRESS = new Tuple<string, int>("192.168.24.2", 6969);
    private Thread clientReceiveThread;

    private AlgoMono _algo;
    #endregion
    // Use this for initialization 	
    public void Init(AlgoMono algo) {
        this._algo = algo;
        ConnectToTcpServer();
    }

    /// <summary> 	
    /// Setup socket connection. 	
    /// </summary> 	
    private void ConnectToTcpServer() {
        try {
            clientReceiveThread = new Thread(new ThreadStart(ListenForData));
            clientReceiveThread.IsBackground = true;
            clientReceiveThread.Start();
        } catch (Exception e) {
            Debug.Log("On client connect exception " + e);
        }
    }
    /// <summary> 	
    /// Runs in background clientReceiveThread; Listens for incomming data. 	
    /// </summary>     
    private void ListenForData() {
        while (true) {
            Debug.Log("Trying to connect to address: " + ADDRESS.Item1 + ":" + ADDRESS.Item2);
            try {
                socketConnection = new TcpClient(ADDRESS.Item1, ADDRESS.Item2);

                //socketConnection = new TcpClient("192.168.24.1", 6969);
                Debug.Log("Connected! ");
                connected = true;
                Byte[] bytes = new Byte[1024];
                while (true) {
                    // Get a stream object for reading 				
                    using (NetworkStream stream = socketConnection.GetStream()) {
                        int length;
                        // Read incomming stream into byte arrary. 					
                        while ((length = stream.Read(bytes, 0, bytes.Length)) != 0) {
                            var incommingData = new byte[length];
                            Array.Copy(bytes, 0, incommingData, 0, length);
                            // Convert byte array to string message. 						
                            string serverMessage = Encoding.ASCII.GetString(incommingData);
                            Debug.Log("Message received as: " + serverMessage);
                            _algo.Receive(serverMessage);
                        }
                    }
                    Thread.Sleep(200);
                }
            } catch (SocketException socketException) {
                connected = false;
                Debug.Log("Socket exception: " + socketException);
                Debug.Log("Reconnecting in 3 seconds. ");
                Thread.Sleep(3000);
            }
        }
    }
    /// <summary> 	
    /// Send message to server using socket connection. 	
    /// </summary> 	
    public void SendMessage(string header, string clientMessage) {
        if (socketConnection == null) {
            return;
        }
        try {
            // Get a stream object for writing. 			
            NetworkStream stream = socketConnection.GetStream();
            if (stream.CanWrite) {
                clientMessage = header + ":" + clientMessage;
                // Convert string message to byte array.
                clientMessage = GetLengthHeader(clientMessage) + clientMessage;
                byte[] clientMessageAsByteArray = Encoding.ASCII.GetBytes(clientMessage);
                // Write byte array to socketConnection stream.                 
                stream.Write(clientMessageAsByteArray, 0, clientMessageAsByteArray.Length);
                Debug.Log("Algo sent message: " + clientMessage);
                stream.Flush();
            }
        } catch (SocketException socketException) {
            Debug.Log("Socket exception: " + socketException);
        }
    }

    string GetLengthHeader(string clientMessage) {
        string len = clientMessage.Length.ToString();
        while (len.Length < 3) {
            len = "0" + len;
        }
        return len;
    }

    public void Disconnect() {
        clientReceiveThread.Abort();
        socketConnection.Close();
    }
}