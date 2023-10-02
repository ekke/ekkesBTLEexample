// (c) 2018 ekke@ekkes-corner.org

package org.ekkescorner.utils;

import android.os.Build;
import java.lang.String;


public class QAndroidUtils 
{
	
	protected QAndroidUtils()
	{
	   //Log.d("ekkescorner", "QAndroidUtils()");
	}
	
	/**
	 *
	*/
    public static String getDeviceModel() {
		// "STH100-2;BlackBerry"
        return Build.MODEL+";"+Build.MANUFACTURER;
    }

}
