package org.example;

import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

public class MathServer {

    private static final String SERVER_IP = "127.0.0.1";
    private static final int SERVER_PORT = 12345;
    private static final int MAX_CLIENTS = 2;

    public static void main(String[] args) {
        new MathServer().start();
    }

    public void start() {
        ExecutorService pool = Executors.newFixedThreadPool(MAX_CLIENTS);
        try (ServerSocket serverSocket = new ServerSocket(SERVER_PORT, MAX_CLIENTS, InetAddress.getByName(SERVER_IP))) {
            System.out.println("Server started on port " + SERVER_PORT);

            int clientCount = getClientCount();
            for (int i = 0; i < clientCount; i++) {
                System.out.println("Waiting for client " + (i + 1));
                Socket clientSocket = serverSocket.accept();
                System.out.println("Client " + (i + 1) + " connected");
                pool.execute(() -> handleClient(clientSocket));
            }

            pool.shutdown();
            pool.awaitTermination(Long.MAX_VALUE, TimeUnit.MILLISECONDS);
            System.out.println("Server shutting down");
        } catch (IOException | InterruptedException e) {
            e.printStackTrace();
        }
    }

    private int getClientCount() {
        Scanner scanner = new Scanner(System.in);
        int input;
        do {
            System.out.print("Please enter a number between 1 and 2: ");
            while (!scanner.hasNextInt()) scanner.next();
            input = scanner.nextInt();
        } while (input < 1 || input > 2);
        return input;
    }

    private void handleClient(Socket socket) {
        try (
                BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                PrintWriter out = new PrintWriter(socket.getOutputStream(), true)
        ) {
            String line;
            while ((line = in.readLine()) != null) {
                System.out.println("Received: " + line);
                String response = processExpression(line);
                out.println(response);
                System.out.println("Sent: " + response);
            }
        } catch (IOException e) {
            System.err.println("Client error: " + e.getMessage());
        }
    }

    private String processExpression(String expr) {
        List<String> tokens = tokenize(expr);
        if (tokens == null || tokens.size() < 3 || !validate(tokens)) {
            return "Incorrect math expression";
        }
        if (!checkDivisionByZero(tokens)) {
            return "Division by zero detected";
        }
        try {
            return "Current expression: " + expr + " = " + calculate(tokens);
        } catch (Exception e) {
            return "Evaluation error: " + e.getMessage();
        }
    }

    private List<String> tokenize(String expr) {
        String[] parts = expr.trim().split("\\s+");
        return Arrays.asList(parts);
    }

    private boolean validate(List<String> tokens) {
        boolean expectNumber = true;
        for (String token : tokens) {
            if (expectNumber) {
                if (!isNumber(token)) return false;
            } else {
                if (!List.of("+", "-", "*", "/").contains(token)) return false;
            }
            expectNumber = !expectNumber;
        }
        return !expectNumber;
    }

    private boolean isNumber(String token) {
        try {
            Double.parseDouble(token);
            return true;
        } catch (NumberFormatException ignored) {
            return false;
        }
    }

    private boolean checkDivisionByZero(List<String> tokens) {
        for (int i = 0; i < tokens.size() - 1; i++) {
            if (tokens.get(i).equals("/") && Double.parseDouble(tokens.get(i + 1)) == 0.0) {
                return false;
            }
        }
        return true;
    }

    private double calculate(List<String> tokens) {
        Stack<Double> values = new Stack<>();
        Stack<String> ops = new Stack<>();

        for (String token : tokens) {
            if (isNumber(token)) {
                values.push(Double.parseDouble(token));
            } else {
                while (!ops.isEmpty() && precedence(ops.peek()) >= precedence(token)) {
                    applyOperation(values, ops.pop());
                }
                ops.push(token);
            }
        }

        while (!ops.isEmpty()) {
            applyOperation(values, ops.pop());
        }

        return values.pop();
    }

    private void applyOperation(Stack<Double> values, String op) {
        double b = values.pop();
        double a = values.pop();
        switch (op) {
            case "+" -> values.push(a + b);
            case "-" -> values.push(a - b);
            case "*" -> values.push(a * b);
            case "/" -> values.push(a / b);
        }
    }

    private int precedence(String op) {
        return switch (op) {
            case "+", "-" -> 1;
            case "*", "/" -> 2;
            default -> 0;
        };
    }
}
