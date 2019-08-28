package com.zhang_ray.easyvoicecall;


import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;


public class Worker implements Runnable {
    static {
        System.loadLibrary("evc-android-opus");
    }

    private native void reInitEncoder(int sampleRate);
    private native void reInitDecoder(int sampleRate);
    private native byte[] encode(byte[] pcm);
    private native byte[] decode(byte[] data);

    private AudioRecord audioRecord;
    private int sampleRate = 16000;
    private int channelInConfig = AudioFormat.CHANNEL_CONFIGURATION_MONO;
    private int audioEncoding = AudioFormat.ENCODING_PCM_16BIT;
    private byte[] buffer = null;
    private byte[] pcmBuffer_mic = new byte[0];
    private byte[] encodedOpusBuffer = new byte[0];

    FileOutputStream opusDumpFile;

    int _10msBufferLength = sampleRate / 100 *2 ;
    private AudioTrack audioTrack = null;


    @Override
    public void run() {

        {
            reInitEncoder(sampleRate);
            reInitDecoder(sampleRate);
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
        audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate, channelInConfig, audioEncoding, audioRecordLength, AudioTrack.MODE_STREAM);

        audioTrack.play();
//        while (isRecording) {
        for (int i = 0; i < 100; i++){
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
                }

                {
                    byte[] tmp = decode(encodedOpusBuffer);
                    audioTrack.write(tmp,0, tmp.length);
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
