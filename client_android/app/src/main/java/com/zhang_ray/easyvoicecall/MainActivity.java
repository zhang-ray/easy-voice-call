package com.zhang_ray.easyvoicecall;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import java.util.Timer;
import java.util.TimerTask;


public class MainActivity extends AppCompatActivity implements SensorEventListener {
    Worker worker;


    static int MSG_CONNECT = 0;

    private static boolean mBackKeyPressed = false;

    Sensor mSensor;
    SensorManager sensorManager;
    PowerManager.WakeLock mWakeLock;
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            updateVolumeMeter((msg.what)/10, (msg.what)%10);
        }
    };



    private void updateVolumeMeter(int inOut, int level){
        if (inOut==0){
//            for (int i = 0; i < level; i++){
//                findViewById(R.id.)
//            }
            ((ProgressBar)findViewById(R.id.pb_volume_mic)).setProgress(level);
        }
        else if (inOut==1){
            ((ProgressBar)findViewById(R.id.pb_volume_spk)).setProgress(level);
        }
    }
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

        ((EditText)findViewById(R.id.et_host)).setText("172.20.122.93");
        ((EditText)findViewById(R.id.et_port)).setText(""+1222);

        ((ToggleButton)(findViewById(R.id.button_connect))).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if(b){
                    if (worker.tryCheckServer(((EditText)findViewById(R.id.et_host)).getText().toString(), Integer.valueOf(((EditText)findViewById(R.id.et_port)).getText().toString()))){
                        {
                            findViewById(R.id.tv_place_phone_to_your_ear).setVisibility(View.VISIBLE);
                            findViewById(R.id.iv_callUp).setVisibility(View.INVISIBLE);
                            findViewById(R.id.iv_hangUp).setVisibility(View.VISIBLE);
                            findViewById(R.id.et_host).setEnabled(false);
                            findViewById(R.id.et_port).setEnabled(false);
                        }

                        worker.asyncStart();
                    }
                    else{
                        // failed
                        ((ToggleButton) (findViewById(R.id.button_connect))).setChecked(false);
                        Toast.makeText(getApplicationContext(),"Invalid server!",Toast.LENGTH_LONG).show();
                    }
                }
                else {
                    if (worker != null) {
                        worker.asyncStop();
                    }
                    {
                        findViewById(R.id.tv_place_phone_to_your_ear).setVisibility(View.INVISIBLE);
                        findViewById(R.id.iv_callUp).setVisibility(View.VISIBLE);
                        findViewById(R.id.iv_hangUp).setVisibility(View.INVISIBLE);
                        findViewById(R.id.et_host).setEnabled(true);
                        findViewById(R.id.et_port).setEnabled(true);
                    }
                }
            }
        });
        ((ToggleButton)(findViewById(R.id.button_connect))).setChecked(false);


        setTitle(getTitle()+" ("+Worker.getVersion() +")");
    }



    void requestPermission(String permission){
        for (;ContextCompat.checkSelfPermission(this, permission)
                != PackageManager.PERMISSION_GRANTED;) {
            ActivityCompat.requestPermissions(this,
                    new String[]{permission}, 0);
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }
    void requestPermissions(){
        requestPermission(Manifest.permission.RECORD_AUDIO);
        requestPermission(Manifest.permission.MODIFY_AUDIO_SETTINGS);
        requestPermission(Manifest.permission.INTERNET);
        requestPermission(Manifest.permission.WAKE_LOCK);
    }

    @Override
    protected void onStart() {
        super.onStart();
        requestPermissions();
        worker = new Worker(this, mHandler);
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

    @Override
    public void onBackPressed() {
        if(!mBackKeyPressed){
            Toast.makeText(this, "Press again to exit the program.", Toast.LENGTH_SHORT).show();
            mBackKeyPressed = true;
            new Timer().schedule(new TimerTask() {
                @Override
                public void run() {
                    mBackKeyPressed = false;
                }
            }, 2000);
        }
        else {//退出程序
            this.finish();
            System.exit(0);
        }
    }
}
