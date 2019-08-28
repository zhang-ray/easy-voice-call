package com.zhang_ray.easyvoicecall;


import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;


public class Worker implements Runnable {

    private AudioRecord audioRecord;
    private int frequence = 48000;
    private int channelInConfig = AudioFormat.CHANNEL_CONFIGURATION_MONO;
    private int audioEncoding = AudioFormat.ENCODING_PCM_16BIT;
    private byte[] buffer = null;


    private AudioTrack track = null;


    @Override
    public void run() {
        int bufferSize = AudioRecord.getMinBufferSize(frequence,
                2, audioEncoding);
        audioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC,
                frequence, channelInConfig, audioEncoding, bufferSize);
        buffer = new byte[bufferSize];

        audioRecord.startRecording();
        boolean isRecording = true;



        bufferSize = AudioTrack.getMinBufferSize(frequence, channelInConfig,
                audioEncoding);
        track = new AudioTrack(AudioManager.STREAM_MUSIC, frequence,
                channelInConfig, audioEncoding, bufferSize,
                AudioTrack.MODE_STREAM);

        track.play();
        while (isRecording) {
            int result = audioRecord.read(buffer, 0, buffer.length);
            track.write(buffer,0, result);
        }

        if (audioRecord != null) {
            audioRecord.stop();
        }
    }
}
