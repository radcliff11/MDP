using System;
using System.Collections;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using UnityEngine;

public class SocketServer : MonoBehaviour {
    public string host = "127.0.0.1";
    public int port = 6969;

    private Algo algoMain;
    public SocketServer(Algo algoMain) {
        this.algoMain = algoMain;
    }
    #region private members 	
    /// <summary> 	
    /// TCPListener to listen for incomming TCP connection 	
    /// requests. 	
    /// </summary> 	
    private TcpListener tcpListener;
    /// <summary> 
    /// Background thread for TcpServer workload. 	
    /// </summary> 	
    private Thread tcpListenerThread;
    /// <summary> 	
    /// Create handle to connected tcp client. 	
    /// </summary> 	
    private TcpClient connectedTcpClient;
    #endregion

    // Use this for initialization
    public void Init() {
        // Start TcpServer background thread 		
        tcpListenerThread = new Thread(new ThreadStart(ListenForIncommingRequests));
        tcpListenerThread.IsBackground = true;
        tcpListenerThread.Start();
    }

    /// <summary> 	
    /// Runs in background TcpServerThread; Handles incomming TcpClient requests 	
    /// </summary> 	
    private void ListenForIncommingRequests() {
        while (true) {
            try {
                // Create listener on localhost port. 	
                Debug.Log(host + port);
                tcpListener = new TcpListener(IPAddress.Parse(host), port);
                tcpListener.Start();
                Debug.Log("Server is listening");
                Byte[] bytes = new Byte[1024];
                using (connectedTcpClient = tcpListener.AcceptTcpClient()) {
                    while (true) {
                        // Get a stream object for reading 					
                        using (NetworkStream stream = connectedTcpClient.GetStream()) {
                            // SendUsernamePassword(); // no need for TCP, username password only for SSH
                            int length;
                            // Read incomming stream into byte arrary. 						
                            while ((length = stream.Read(bytes, 0, bytes.Length)) != 0) {
                                var incommingData = new byte[length];
                                Array.Copy(bytes, 0, incommingData, 0, length);
                                // Convert byte array to string message. 							
                                string clientMessage = Encoding.ASCII.GetString(incommingData);
                                Debug.Log("client message received as: " + clientMessage);

                                //_algo.Receive(clientMessage);
                            }
                        }
                        Thread.Sleep(100);
                    }
                }
            } catch (SocketException socketException) {
                Debug.Log("SocketException " + socketException.ToString());
                Debug.Log("Reconnecting in 3 seconds. ");
                Thread.Sleep(3000);
            }
        }
    }
    /// <summary> 	
    /// Send message to client using socket connection. 	
    /// </summary> 	
    public new void SendMessage(string outgoingMsg) {
        if (connectedTcpClient == null) { return; }

        try {
            // Get a stream object for writing. 			
            NetworkStream stream = connectedTcpClient.GetStream();
            if (stream.CanWrite) {
                // Convert string message to byte array.                 
                byte[] serverMessageAsByteArray = Encoding.ASCII.GetBytes(outgoingMsg);
                // Write byte array to socketConnection stream.               
                stream.Write(serverMessageAsByteArray, 0, serverMessageAsByteArray.Length);
                Debug.Log("Server sent his message - should be received by client");
            }
        } catch (SocketException socketException) {
            Debug.Log("Socket exception: " + socketException);
        }
    }

}

