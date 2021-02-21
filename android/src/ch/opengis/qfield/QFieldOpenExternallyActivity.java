package ch.opengis.qfield;

import java.io.File;

import android.os.Bundle;
import android.os.Build;
import android.os.Environment;
import android.net.Uri;
import android.app.Activity;
import android.content.Intent;
import android.util.Log;
import androidx.core.content.FileProvider;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.FileInputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.util.Date;
import java.text.SimpleDateFormat;

public class QFieldOpenExternallyActivity extends Activity{
    private static final String TAG = "QField Open (file) Externally Activity";
    private String filePath;
    private String mimeType;
    private String tempFileName;
    private String errorMessage;

    @Override
    protected void onCreate(Bundle savedInstanceState){
        Log.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);

        filePath = getIntent().getExtras().getString("filepath");
        mimeType = getIntent().getExtras().getString("filetype");
        Log.d(TAG, "Received filepath: " + filePath + " and mimeType: " + mimeType);

        File file = new File(filePath);
        File cacheFile = new File(getCacheDir(), file.getName());

        //copy file to a temporary file
        try{
            copyFile( file, cacheFile );
        }catch(IOException e){
            Log.d(TAG, e.getMessage());
            finish();
        }
      
        Uri contentUri =  Build.VERSION.SDK_INT < 24 ? Uri.fromFile(file) : FileProvider.getUriForFile( this, BuildConfig.APPLICATION_ID+".fileprovider", cacheFile );

        Log.d(TAG, "content URI: " + contentUri);
        Log.d(TAG, "call ACTION_VIEW intent");
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_VIEW);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        intent.setDataAndType(contentUri, mimeType);
        try{
            startActivityForResult(intent, 102);
        }catch (Exception e) {
            Log.d(TAG, e.getMessage());
            errorMessage = e.getMessage();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) 
    {
      //on ACTION_VIEW back key pressed it returns RESULT_CANCEL - on error as well
      if (resultCode == RESULT_OK) {
          Intent intent = this.getIntent();
          setResult(RESULT_OK, intent);
      } else {
          Intent intent = this.getIntent();
          intent.putExtra("ERROR_MESSAGE", errorMessage);
          setResult(RESULT_CANCELED, intent);
      }

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
