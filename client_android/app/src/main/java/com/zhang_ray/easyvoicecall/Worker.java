package com.zhang_ray.easyvoicecall;


import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.os.Build;
import android.os.PowerManager;
import android.util.Log;

import androidx.annotation.NonNull;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;


public class Worker implements Runnable {
    static {
        System.loadLibrary("evc-android-opus");
        System.loadLibrary("client_essential");
    }


    /// from NetPacket.hpp
    static int Undefined = 0;
    static int HeartBeatRequest = 1;
    static int HeartBeatResponse = 2;
    static int LoginRequest = 3;
    static int LoginResponse = 4;
    static int UserInfo = 5;
    static int TextMessage = 6;
    static int AudioMessage = 7;
    static int AudioMessage_CN = 8;
    static int LogoutRequest = 9;
    static int LogoutResponse = 10;
    /// from NetPacket.hpp


    static int audioSN = 0;


    public static String TAG = "EVC";

    public native static String getVersion();

    private native void reInitEncoder(int sampleRate);
    private native void reInitDecoder(int sampleRate);
    private native byte[] encode(byte[] pcm);
    private native byte[] decode(byte[] data);
    private native byte[] createNetPacket(int type);
    private native byte[] createNetPacketWithSN(int type, int sn);
    private native byte[] createNetPacketWithPayload(int type, int sn, byte[] data);
    private native int getNetPacketType(byte[] raw);
    private native byte[] getNetPacketPayload(byte[] raw);

    private AudioRecord audioRecord;
    private int sampleRate = 16000;
    private int channelInConfig = AudioFormat.CHANNEL_CONFIGURATION_MONO;
    private int audioEncoding = AudioFormat.ENCODING_PCM_16BIT;
    private byte[] buffer = null;
    private byte[] pcmBuffer_mic = new byte[0];
    private byte[] encodedOpusBuffer = new byte[0];
    private byte[] receivedBuffer = new byte[100000];

    FileOutputStream opusDumpFile;

    int _10msBufferLength = sampleRate / 100 *2 ;
    private AudioTrack audioTrack = null;
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        DatagramSocket datagramSocket = null;

    DatagramSocket udpSocket;

    String finalIP ="172.20.122.93";
    int finalPort = 1222;

    AudioManager audioManager;
    Context context;

    public Worker(@NonNull Context context){
        this.context = context;
    }

    private void initAudioManager() {
        audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        audioManager.setSpeakerphoneOn(false);
        audioManager.setMode(AudioManager.MODE_IN_COMMUNICATION);
    }



    @Override
    public void run() {

        initAudioManager();

        {
            reInitEncoder(sampleRate);
            reInitDecoder(sampleRate);
        }

        {

            try {
                udpSocket = new DatagramSocket();

                byte[] dataToSend = createNetPacket(LoginRequest);
                udpSocket.send(new DatagramPacket(dataToSend, dataToSend.length, InetAddress.getByName(finalIP), finalPort));

                DatagramPacket datagramPacket = new DatagramPacket(receivedBuffer, receivedBuffer.length, InetAddress.getByName(finalIP), finalPort);
                udpSocket.receive(datagramPacket);
                int theLength = datagramPacket.getLength();
                Log.d("EVC", "received(byte) " + theLength);
            } catch (SocketException e) {
                e.printStackTrace();
            } catch (UnknownHostException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }

        try {
            opusDumpFile=new FileOutputStream("/sdcard/dump.opus");
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        int audioRecordLength = AudioRecord.getMinBufferSize(sampleRate,2, audioEncoding);
        audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, sampleRate, channelInConfig, audioEncoding, audioRecordLength);
        buffer = new byte[audioRecordLength];

        audioRecord.startRecording();
        boolean isRecording = true;

        audioRecordLength = AudioTrack.getMinBufferSize(sampleRate, channelInConfig, audioEncoding);
        audioTrack = new AudioTrack(AudioManager.STREAM_VOICE_CALL, sampleRate, channelInConfig, audioEncoding, audioRecordLength, AudioTrack.MODE_STREAM);

        audioTrack.play();
        while (isRecording) {
//        for (int i = 0; i < 100; i++){
            int pcmLengthInByte_mic = audioRecord.read(buffer, 0, buffer.length);
            //audioTrack.write(buffer,0, result);

            {
                byte[] tempArray = new byte[pcmBuffer_mic.length + pcmLengthInByte_mic];
                System.arraycopy(pcmBuffer_mic, 0, tempArray, 0, pcmBuffer_mic.length);
                System.arraycopy(buffer, 0, tempArray, pcmBuffer_mic.length, pcmLengthInByte_mic);
                pcmBuffer_mic = tempArray;
            }


            for( ;pcmBuffer_mic.length >= _10msBufferLength; ){

                byte[] _10msBuffer = new byte[_10msBufferLength];
                System.arraycopy(pcmBuffer_mic, 0, _10msBuffer, 0, _10msBuffer.length);

                {
                    encodedOpusBuffer = encode(_10msBuffer);
                    try {
                        byte[] dataToSend = createNetPacketWithPayload(AudioMessage,audioSN, encodedOpusBuffer);
                        udpSocket.send(new DatagramPacket(dataToSend, dataToSend.length, InetAddress.getByName(finalIP), finalPort));
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }

                {
                    try {
                        DatagramPacket datagramPacket = new DatagramPacket(receivedBuffer, receivedBuffer.length, InetAddress.getByName(finalIP), finalPort);
                        udpSocket.receive(datagramPacket);

                        byte[] realData = new byte[datagramPacket.getLength()];
                        System.arraycopy(receivedBuffer, 0 , realData, 0, datagramPacket.getLength());

                        if (getNetPacketType(realData)==AudioMessage){
                            byte[] payload = getNetPacketPayload(realData);
                            byte[] decodedPcm = decode(payload);
                            audioTrack.write(decodedPcm, 0, decodedPcm.length);
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }

                {
                    byte[] tempArray = new byte[pcmBuffer_mic.length - _10msBuffer.length];
                    System.arraycopy(pcmBuffer_mic, _10msBuffer.length, tempArray, 0, tempArray.length);
                    pcmBuffer_mic = tempArray;
                }
            }
        }

        if (audioRecord != null) {
            audioRecord.stop();
        }

        if (audioTrack != null){
            audioTrack.stop();
        }

        try {
            opusDumpFile.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
