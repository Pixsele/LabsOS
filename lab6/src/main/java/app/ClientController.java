package app;

import javafx.fxml.FXML;
import javafx.scene.control.TextArea;
import javafx.scene.control.TextField;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

public class ClientController {

    private static final String SERVER_IP = "127.0.0.1";
    private static final int SERVER_PORT = 12345;
    private Socket socket;
    private BufferedReader in;
    private PrintWriter out;

    @FXML
    private TextArea status;
    @FXML
    private TextField inputExpr;
    @FXML
    private TextArea answer;


    @FXML
    public void initialize() throws IOException {
        status.setEditable(false);
        try{
            socket = new Socket(SERVER_IP, SERVER_PORT);
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = new PrintWriter(socket.getOutputStream(), true);

            status.setText("Status: Online");
        }catch(IOException e){
            status.setText("Status: Offline");
        }
    }

    @FXML
    private void sendToServer(){
        String input = inputExpr.getText();
        if(input.isEmpty()) return;

        out.println(input);

        try{
            String response = in.readLine();
            answer.setText(input + " = " + response);

        }catch(IOException e){
            answer.setText("Error");
        }
    }

}
