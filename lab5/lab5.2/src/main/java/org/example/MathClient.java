package org.example;


import java.io.*;
import java.net.*;
import java.util.Scanner;

public class MathClient{

    private static final String SERVER_IP = "127.0.0.1";
    private static final int SERVER_PORT = 12345;


    public static void main(String[] args) {

        try (Socket socket = new Socket(SERVER_IP, SERVER_PORT);
             BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
             PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
             Scanner scanner = new Scanner(System.in)) {


            System.out.println("Connected to server.");


            String index = in.readLine();
            System.out.println("Client#: " + index);
            while (true) {
                System.out.print("Enter expression: ");
                String input = scanner.nextLine();
                out.println(input);
                String response = in.readLine();

                if (response == null) {
                    System.out.println("Server disconnected.");
                    break;
                }

                System.out.println("Server: " + response);
            }

        } catch (IOException e) {
            System.err.println("Connection failed: " + e.getMessage());
        }
    }
}
