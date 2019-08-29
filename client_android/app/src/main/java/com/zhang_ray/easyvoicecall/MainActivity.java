package com.zhang_ray.easyvoicecall;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.PowerManager;
import android.view.View;


public class MainActivity extends AppCompatActivity implements SensorEventListener {
    Worker worker = new Worker(this);

    Sensor mSensor;
    SensorManager sensorManager;
    PowerManager.WakeLock mWakeLock;

    @Override
    protected void onResume() {
        super.onResume();

        if (mSensor != null) {
            sensorManager.registerListener(this, mSensor, SensorManager.SENSOR_DELAY_NORMAL);
        }
    }

    private void initWakeLock(){
        sensorManager = (SensorManager) this.getSystemService(Context.SENSOR_SERVICE);
        mSensor = sensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);

        PowerManager mPowerManager = (PowerManager) this.getSystemService(Context.POWER_SERVICE);
        mWakeLock = mPowerManager.newWakeLock(PowerManager.PROXIMITY_SCREEN_OFF_WAKE_LOCK, Worker.TAG);
    }


    void initView(){
        ((android.widget.Button)findViewById(R.id.button_connect)).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                (new Thread(worker)).start();
            }
        });

        ((android.widget.TextView)findViewById(R.id.tv_so_version)).setText(Worker.getVersion());
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        initView();
        initWakeLock();
    }

    @Override
    public void onSensorChanged(SensorEvent sensorEvent) {
        if (sensorEvent.values[0] <= 0.1) {
            // close to the phone
            if (!mWakeLock.isHeld()) {
                mWakeLock.acquire();
            }
        } else {
            // far away from phone
            // wake up
            if (mWakeLock.isHeld())
                mWakeLock.release();
        }

    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {

    }
}
