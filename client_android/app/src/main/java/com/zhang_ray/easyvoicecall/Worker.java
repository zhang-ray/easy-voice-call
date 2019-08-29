package com.zhang_ray.easyvoicecall;


import android.content.Context;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.os.Message;
import android.util.Log;

import androidx.annotation.NonNull;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;

import android.os.Handler;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class Worker implements Runnable {
    static {
        System.loadLibrary("evc-android-opus");
        System.loadLibrary("client_essential");
    }


    /// from NetPacket.hpp
    private static int Undefined = 0;
    private static int HeartBeatRequest = 1;
    private static int HeartBeatResponse = 2;
    private static int LoginRequest = 3;
    private static int LoginResponse = 4;
    private static int UserInfo = 5;
    private static int TextMessage = 6;
    private static int AudioMessage = 7;
    private static int AudioMessage_CN = 8;
    private static int LogoutRequest = 9;
    private static int LogoutResponse = 10;
    /// from NetPacket.hpp


    private static int audioSN = 0;
    private static int counterVolumeMeter_mic = 0;
    private static int counterVolumeMeter_spk = 0;
    private static final int counterCycle = 2;


    static String TAG = "EVC";

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

    // return recently max volume level , 0 to 9
    private native short calcVolume(byte[]pcm, int inOut);

    private int sampleRate = 16000;
    private byte[] pcmBuffer_mic = new byte[0];
    private byte[] receivedBuffer = new byte[100000];

    private int _10msBufferLength = sampleRate / 100 *2 ;
    private AudioTrack audioTrack = null;


    private boolean keepWorking = true;
    int channelInConfig = AudioFormat.CHANNEL_CONFIGURATION_MONO;
    int audioEncoding = AudioFormat.ENCODING_PCM_16BIT;
    int audioRecordLength = AudioRecord.getMinBufferSize(sampleRate,2, audioEncoding);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        DatagramSocket datagramSocket = null;

    private DatagramSocket udpSocket;

    private String finalIP;
    private int finalPort;

    private AudioManager audioManager;
    private Context context;
    private Handler handler;

    Worker(@NonNull Context context, @NonNull Handler handler){
        this.context = context;
        this.handler = handler;
    }

    private static boolean isIpv4(String ipAddress) {
        String ip = "^(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|[1-9])\\."
                + "(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)\\."
                + "(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)\\."
                + "(1\\d{2}|2[0-4]\\d|25[0-5]|[1-9]\\d|\\d)$";
        Pattern pattern = Pattern.compile(ip);
        Matcher matcher = pattern.matcher(ipAddress);
        return matcher.matches();
    }



    private void initAudioManager() {
        audioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        audioManager.setSpeakerphoneOn(false);
        audioManager.setMode(AudioManager.MODE_IN_COMMUNICATION);
    }



    private class DownStreamThread implements Runnable {

        @Override
        public void run(){
            try {
                audioTrack = new AudioTrack(AudioManager.STREAM_VOICE_CALL, sampleRate, channelInConfig, audioEncoding, audioRecordLength, AudioTrack.MODE_STREAM);
                audioTrack.play();

                for (;keepWorking;) {
                    DatagramPacket datagramPacket = new DatagramPacket(receivedBuffer, receivedBuffer.length, InetAddress.getByName(finalIP), finalPort);
                    udpSocket.receive(datagramPacket);

                    byte[] realData = new byte[datagramPacket.getLength()];
                    System.arraycopy(receivedBuffer, 0, realData, 0, datagramPacket.getLength());

                    if (getNetPacketType(realData) == AudioMessage) {
                        byte[] payload = getNetPacketPayload(realData);
                        byte[] decodedPcm = decode(payload);
                        audioTrack.write(decodedPcm, 0, decodedPcm.length);

                        if (counterVolumeMeter_spk++ % counterCycle==0) {
                            handler.sendEmptyMessage(calcVolume(decodedPcm, 1) + 10);
                        }
                    }
                }
                if (audioTrack != null){
                    audioTrack.stop();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
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

                (new Thread(new DownStreamThread())).start();

            } catch (SocketException e) {
                e.printStackTrace();
            } catch (UnknownHostException e) {
                e.printStackTrace();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }


        AudioRecord audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, sampleRate, channelInConfig, audioEncoding, audioRecordLength);
        byte[] buffer = new byte[audioRecordLength];

        audioRecord.startRecording();

        audioRecordLength = AudioTrack.getMinBufferSize(sampleRate, channelInConfig, audioEncoding);


        while (keepWorking) {
            int pcmLengthInByte_mic = audioRecord.read(buffer, 0, buffer.length);
            if (pcmLengthInByte_mic <= 0){
                Log.e("EVC", "pcmLengthInByte_mic " + pcmLengthInByte_mic);
                continue;
            }

            {
                byte[] tempArray = new byte[pcmBuffer_mic.length + pcmLengthInByte_mic];
                System.arraycopy(pcmBuffer_mic, 0, tempArray, 0, pcmBuffer_mic.length);
                System.arraycopy(buffer, 0, tempArray, pcmBuffer_mic.length, pcmLengthInByte_mic);
                pcmBuffer_mic = tempArray;
            }


            for( ;pcmBuffer_mic.length >= _10msBufferLength; ){

                byte[] _10msBuffer = new byte[_10msBufferLength];
                System.arraycopy(pcmBuffer_mic, 0, _10msBuffer, 0, _10msBuffer.length);


                if (counterVolumeMeter_mic++ % counterCycle==0) {
                    handler.sendEmptyMessage(calcVolume(_10msBuffer, 0) + 0);
                }

                {
                    byte[] encodedOpusBuffer = encode(_10msBuffer);
                    try {
                        byte[] dataToSend = createNetPacketWithPayload(AudioMessage,audioSN++, encodedOpusBuffer);
                        udpSocket.send(new DatagramPacket(dataToSend, dataToSend.length, InetAddress.getByName(finalIP), finalPort));
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

        audioRecord.stop();

    }



    boolean tryCheckServer(String ip, int port){
        if (!isIpv4(ip)) {
            return false;
        }

        this.finalIP = ip;
        this.finalPort = port;

        return true;
    }

    void asyncStart() {
        keepWorking = true;
        (new Thread(this)).start();
    }

    void asyncStop(){
        keepWorking=false;
    }

}
