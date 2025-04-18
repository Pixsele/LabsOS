package app;

import javafx.application.Application;
import javafx.fxml.FXMLLoader;
import javafx.scene.Scene;
import javafx.stage.Stage;

import java.io.IOException;

public class Client extends Application {

    @Override
    public void start(Stage stage) throws IOException {
        FXMLLoader fxmlLoader = new FXMLLoader(getClass().getResource("client.fxml"));
        Scene scene = new Scene(fxmlLoader.load());

        String index = getParameters().getRaw().getFirst();

        stage.setTitle("Client " + index);
        stage.setScene(scene);
        stage.setResizable(false);
        stage.show();
    }

    public static void main(String[] args) {
        String index = args[0];
        System.out.println(index);
        launch(args);
    }
}
