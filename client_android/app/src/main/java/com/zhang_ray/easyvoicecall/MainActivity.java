package com.zhang_ray.easyvoicecall;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;


public class MainActivity extends AppCompatActivity {

    Worker worker = new Worker();



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ((android.widget.Button)findViewById(R.id.button_connect)).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                (new Thread(worker)).start();
            }
        });

        ((android.widget.TextView)findViewById(R.id.tv_so_version)).setText(Worker.getVersion());
    }
}
