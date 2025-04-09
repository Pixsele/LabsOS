package app;

import javafx.fxml.FXML;
import javafx.scene.control.TextField;
import javafx.stage.Stage;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

import static java.lang.System.exit;

public class ClientController {

    private static final String SERVER_IP = "127.0.0.1";
    private static final int SERVER_PORT = 12345;
    private static Socket socket;
    private BufferedReader in;
    private PrintWriter out;

    @FXML
    private TextField status;
    @FXML
    private TextField inputExpr;

    private boolean connected;

    @FXML
    public void initialize() {

        status.setEditable(false);
        try{
            socket = new Socket(SERVER_IP, SERVER_PORT);
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            out = new PrintWriter(socket.getOutputStream(), true);

            status.setText("Status: Online");
            connected = true;

        }catch(IOException e){
            status.setText("Status: Offline");
            connected = false;
        }

    }

    @FXML
    private void sendToServer() throws InterruptedException {
        if(connected){
            String input = inputExpr.getText();
            if(input.isEmpty()) return;

            out.println(input);

            try{
                String response = in.readLine();
                inputExpr.setText(response);
            }catch(IOException e){
                status.setText("Status: Offline, try to restart");
            }
        }
    }

    @FXML
    private void clearInput(){
        inputExpr.setText("");
    }

    @FXML
    private void addDigit(String digit) {
        String input = inputExpr.getText();
        input = input + digit;
        inputExpr.setText(input);
    }

    @FXML
    private void addDot(){
        addDigit(".");
    }

    @FXML
    private void addOne() {
        addDigit("1");
    }

    @FXML
    private void printName(){
        status.setText("Kirill Vorobyev");
    }

    @FXML
    private void addTwo() {
        addDigit("2");
    }

    @FXML
    private void addThree() {
        addDigit("3");
    }

    @FXML
    private void addFour() {
        addDigit("4");
    }

    @FXML
    private void addFive() {
        addDigit("5");
    }

    @FXML
    private void addSix() {
        addDigit("6");
    }

    @FXML
    private void addSeven() {
        addDigit("7");
    }

    @FXML
    private void addEight() {
        addDigit("8");
    }

    @FXML
    private void addNine() {
        addDigit("9");
    }

    @FXML
    private void addZero() {
        addDigit("0");
    }

    @FXML
    private void addPlus(){
        String input = inputExpr.getText();
        input = input + " + ";
        inputExpr.setText(input);
    }

    @FXML
    private void addMinus(){
        String input = inputExpr.getText();
        input = input + " - ";
        inputExpr.setText(input);
    }

    @FXML
    private void addMultiply(){
        String input = inputExpr.getText();
        input = input + " * ";
        inputExpr.setText(input);
    }

    @FXML
    private void addDivide(){
        String input = inputExpr.getText();
        input = input + " / ";
        inputExpr.setText(input);
    }

}
