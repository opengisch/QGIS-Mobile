package ch.opengis.qfield;

import java.text.SimpleDateFormat;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.util.Date;

import android.os.Bundle;
import android.os.Environment;
import android.net.Uri;
import android.app.Activity;
import android.content.Intent;
import android.util.Log;
import android.provider.MediaStore;
import android.graphics.Bitmap;
import androidx.core.content.FileProvider;
import android.media.MediaScannerConnection;

public class QFieldCameraPictureActivity extends Activity{
    private static final String TAG = "QField Camera Picture Activity";
    private String prefix;
    private String pictureFilePath;
    private String suffix;
    private String pictureTempFileName;

    @Override
    protected void onCreate(Bundle savedInstanceState){
        Log.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        prefix = getIntent().getExtras().getString("prefix");
        pictureFilePath = getIntent().getExtras().getString("pictureFilePath");
        suffix = getIntent().getExtras().getString("suffix");
        Log.d(TAG, "Received prefix: " + prefix +" and pictureFilePath: " + pictureFilePath + "and suffix: " + suffix);

        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        pictureTempFileName = "QFieldPicture" + timeStamp + '.' +suffix;
        Log.d(TAG, "Created pictureTempFileName: " + pictureTempFileName);

        callCameraIntent();

        return;
    }

    private void callCameraIntent() {
        Log.d(TAG, "callCameraIntent()");
        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        if (takePictureIntent.resolveActivity(getPackageManager()) != null) {
            File pictureFile = new File(getCacheDir(), pictureTempFileName);

            if (pictureFile != null) {

                Uri photoURI = FileProvider.getUriForFile(this,
                                                          "ch.opengis.qfield.fileprovider",
                                                          pictureFile);

                takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, photoURI);
                startActivityForResult(takePictureIntent, 172);
            }
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.d(TAG, "onActivityResult()");
        Log.d(TAG, "resultCode: "+resultCode);

        File result = new File(prefix+pictureFilePath);
        File path = result.getParentFile();
        path.mkdirs();

        // Let the android scan new media folders/files to make them visible through MTP
        path.setExecutable(true);
        path.setReadable(true);
        path.setWritable(true);
        MediaScannerConnection.scanFile(this, new String[] {path.toString()}, null, null);

        if (resultCode == RESULT_OK) {

            File pictureFile = new File(getCacheDir(), pictureTempFileName);
            Log.d(TAG, "Taken picture: " + pictureFile.getAbsolutePath());
            try{
                copyFile(pictureFile, result);
            }catch(IOException e){
                Log.d(TAG, e.getMessage());
            }

            Intent intent = this.getIntent();
            intent.putExtra("PICTURE_IMAGE_FILENAME", prefix+pictureFilePath);
            setResult(RESULT_OK, intent);
        } else {
            Intent intent = this.getIntent();
            intent.putExtra("PICTURE_IMAGE_FILENAME", "");
            setResult(RESULT_CANCELED, intent);
        }

        // Let the android scan new media folders/files to make them visible through MTP
        result.setReadable(true);
        result.setWritable(true);
        MediaScannerConnection.scanFile(this, new String[] {result.toString()}, null, null);

        finish();
    }

    private void copyFile(File src, File dst) throws IOException {
        Log.d(TAG, "Copy file: "+src.getAbsolutePath()+" to file: "+dst.getAbsolutePath());
        try (InputStream in = new FileInputStream(src)) {
            try (OutputStream out = new FileOutputStream(dst)) {
                // Transfer bytes from in to out
                byte[] buf = new byte[1024];
                int len;
                while ((len = in.read(buf)) > 0) {
                    out.write(buf, 0, len);
                }
                out.close();
            }
        }
    }

}
