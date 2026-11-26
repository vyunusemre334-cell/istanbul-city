package com.istanbulvice.underground;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.widget.FrameLayout;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public final class MainActivity extends Activity {
    static { System.loadLibrary("istanbulvice"); }
    private GLSurfaceView surface;
    private HudView hud;
    private int savedStage;
    private int savedMissionId;

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        savedStage = getPreferences(MODE_PRIVATE).getInt("mission_stage", 0);
        savedMissionId = getPreferences(MODE_PRIVATE).getInt("mission_id", 1);
        FrameLayout root = new FrameLayout(this);
        surface = new GLSurfaceView(this);
        surface.setEGLContextClientVersion(3);
        surface.setPreserveEGLContextOnPause(true);
        surface.setRenderer(new GLSurfaceView.Renderer() {
            @Override public void onSurfaceCreated(GL10 gl, EGLConfig config) { nativeInit(savedStage, savedMissionId); }
            @Override public void onSurfaceChanged(GL10 gl, int w, int h) { nativeResize(w, h); }
            @Override public void onDrawFrame(GL10 gl) { nativeFrame(); }
        });
        surface.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        hud = new HudView(this);
        root.addView(surface, new FrameLayout.LayoutParams(-1, -1));
        root.addView(hud, new FrameLayout.LayoutParams(-1, -1));
        setContentView(root);
    }

    @Override protected void onPause() {
        if (hud != null) saveProgress(nativeGetMissionStage(), nativeGetMissionId());
        super.onPause(); if (surface != null) surface.onPause();
    }
    @Override protected void onResume() { super.onResume(); if (surface != null) surface.onResume(); }
    private void saveProgress(int stage, int missionId) { savedStage = stage; savedMissionId = missionId; getPreferences(MODE_PRIVATE).edit().putInt("mission_stage", stage).putInt("mission_id", missionId).apply(); }

    private final class HudView extends View {
        private final Paint paint = new Paint(3);
        private int joystickPointer = -1;
        private float joyX, joyY, joyRadius;
        private final int[] buttonPointers = {-1, -1, -1, -1, -1};
        private final float[] bx = {.78f, .91f, .72f, .90f, .90f};
        private final float[] by = {.53f, .82f, .82f, .62f, .43f};
        private final String[] labels = {"KOŞ", "ZIPLA", "EĞİL", "ETK", "GÖREV"};
        private boolean missionVisible = true;
        private final float[] radii = {.060f, .060f, .060f, .060f, .060f};
        private int lastSaved = -1;
        private int lastSavedMission = -1;
        HudView(Context c) { super(c); setLayerType(View.LAYER_TYPE_HARDWARE, null); setClickable(true); }

        @Override protected void onDraw(Canvas c) {
            super.onDraw(c);
            float w = getWidth(), h = getHeight(), unit = Math.min(w, h);
            joyX = w * .16f; joyY = h * .79f; joyRadius = unit * .115f;
            paint.setColor(0x55334455); paint.setStyle(Paint.Style.FILL); c.drawCircle(joyX, joyY, joyRadius, paint);
            paint.setColor(0x99DDEEFF); paint.setStyle(Paint.Style.STROKE); paint.setStrokeWidth(unit * .006f); c.drawCircle(joyX, joyY, joyRadius, paint);
            paint.setStyle(Paint.Style.FILL); paint.setColor(0x88DDEEFF); c.drawCircle(joyX, joyY, unit * .035f, paint);
            for (int i=0;i<5;i++) {
                float x=w*bx[i], y=h*by[i], r=unit*radii[i];
                paint.setStyle(Paint.Style.FILL); paint.setColor(buttonPointers[i]>=0?0xBB7A45D6:0x77505D78); c.drawCircle(x,y,r,paint);
                paint.setStyle(Paint.Style.STROKE); paint.setStrokeWidth(unit*.005f); paint.setColor(0xCCDDEEFF); c.drawCircle(x,y,r,paint);
                paint.setStyle(Paint.Style.FILL); paint.setColor(0xFFFFFFFF); paint.setTextAlign(Paint.Align.CENTER); paint.setTypeface(Typeface.create("sans-serif",Typeface.BOLD)); paint.setTextSize(unit*.021f); c.drawText(i==3?(nativeGetDriving()?"İN/ÇIK":"ETK"):labels[i],x,y+unit*.007f,paint);
            }
            int stage=nativeGetMissionStage(), missionId=nativeGetMissionId(); if(stage!=lastSaved || missionId!=lastSavedMission){lastSaved=stage;lastSavedMission=missionId;saveProgress(stage, missionId);}
            paint.setTextAlign(Paint.Align.LEFT); paint.setTypeface(Typeface.create("sans-serif",Typeface.BOLD)); paint.setTextSize(unit*.026f); paint.setColor(0xFFFFFFFF);
            String[] tasks={"Bölgeye ilerle", "Bölgedeki etkileşimi tamamla", "Güvenli hedefe ulaş", "Görev tamamlandı"};
            String[] missionNames={"MOR BÖLGE", "SAHİL OPERASYONU", "KÖPRÜ HATTI"};
            String missionName=missionNames[Math.max(0,Math.min(2,missionId-1))];
            c.drawText("İSTANBUL CITY  •  "+missionName+"  •  FPS "+Math.round(nativeGetFps()),unit*.035f,unit*.055f,paint);
            paint.setTextSize(unit*.021f); c.drawText("CAN: "+Math.round(nativeGetHealth())+(nativeGetDriving()?"   HIZ: "+Math.round(Math.abs(nativeGetSpeed())*3.6f)+" KM/H":"")+"   GÖREV: "+tasks[Math.max(0,Math.min(3,stage))],unit*.035f,unit*.09f,paint);
            if (missionVisible) {
                float left=unit*.035f, top=unit*.115f, right=left+unit*.47f, bottom=top+unit*.105f;
                paint.setColor(0xAA101827); paint.setStyle(Paint.Style.FILL); c.drawRoundRect(left,top,right,bottom,unit*.012f,unit*.012f,paint);
                paint.setColor(0xFFFFFFFF); paint.setTextSize(unit*.019f); c.drawText("GÖREV  •  İSTANBUL CITY",left+unit*.014f,top+unit*.032f,paint);
                paint.setTextSize(unit*.016f); c.drawText(stage==0?"İşaretli görev bölgesine ilerle.":stage==1?"Bölgeye ulaştın. ETK düğmesine bas.":stage==2?"Haritadaki altın işarete ulaş ve izini kaybettir.":"Görev tamamlandı. GÖREV düğmesiyle sıradakine geç.",left+unit*.014f,top+unit*.071f,paint);
            }
            postInvalidateDelayed(300);
        }

        private int hitButton(float x,float y){float unit=Math.min(getWidth(),getHeight());for(int i=0;i<5;i++){float dx=x-getWidth()*bx[i],dy=y-getHeight()*by[i];if(dx*dx+dy*dy<(unit*radii[i]*1.35f)*(unit*radii[i]*1.35f))return i;}return -1;}
        private void updateJoystick(float x,float y){float dx=x-joyX,dy=y-joyY,limit=joyRadius;float len=(float)Math.hypot(dx,dy);if(len>limit){dx*=limit/len;dy*=limit/len;}nativeMove(dx/limit,-dy/limit);invalidate();}
        @Override public boolean onTouchEvent(MotionEvent e){int action=e.getActionMasked(),index=e.getActionIndex(),id=e.getPointerId(index);float x=e.getX(index),y=e.getY(index);
            if(action==MotionEvent.ACTION_DOWN||action==MotionEvent.ACTION_POINTER_DOWN){int b=hitButton(x,y);if(b>=0){buttonPointers[b]=id;if(b==4 && nativeGetMissionStage()!=3)missionVisible=!missionVisible;nativeButton(b,e.getAction()!=MotionEvent.ACTION_CANCEL);invalidate();return true;}if(x<getWidth()*.43f&&joystickPointer<0){joystickPointer=id;updateJoystick(x,y);return true;}return true;}
            if(action==MotionEvent.ACTION_MOVE){if(joystickPointer>=0){int i=e.findPointerIndex(joystickPointer);if(i>=0)updateJoystick(e.getX(i),e.getY(i));}return true;}
            if(action==MotionEvent.ACTION_UP||action==MotionEvent.ACTION_POINTER_UP||action==MotionEvent.ACTION_CANCEL){if(id==joystickPointer){joystickPointer=-1;nativeMove(0,0);}for(int i=0;i<buttonPointers.length;i++)if(buttonPointers[i]==id){buttonPointers[i]=-1;nativeButton(i,false);}invalidate();return true;}return true;
        }
    }

    private static native void nativeInit(int savedStage, int savedMissionId);
    private static native void nativeResize(int width,int height);
    private static native void nativeFrame();
    private static native void nativeMove(float x,float y);
    private static native void nativeButton(int code,boolean down);
    private static native int nativeGetMissionStage();
    private static native int nativeGetMissionId();
    private static native float nativeGetFps();
    private static native float nativeGetHealth();
    private static native boolean nativeGetDriving();
    private static native float nativeGetSpeed();
}
