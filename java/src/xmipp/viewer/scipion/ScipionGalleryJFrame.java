/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package xmipp.viewer.scipion;

import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.io.File;
import java.util.HashMap;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JButton;
import xmipp.ij.commons.XmippUtil;
import xmipp.jni.Filename;
import xmipp.jni.MetaData;
import xmipp.utils.XmippDialog;
import xmipp.utils.XmippWindowUtil;
import xmipp.viewer.windows.GalleryJFrame;

/**
 *
 * @author airen
 */
public class ScipionGalleryJFrame extends GalleryJFrame {

    private String type;
    private String script, ctfscript;
    private String projectid;
    private JButton cmdbutton;
    private String sqlitefile;
    private String outputdir;
    private JButton classcmdbutton;
    private String python;
    private String inputid;
    private HashMap<String, String> msgfields;
    private final String runNameKey = "Run name:";
    private String other;
    
   

    public ScipionGalleryJFrame(String filename, ScipionMetaData md, ScipionParams parameters) {
        super(filename, md, parameters);
        readScipionParams(parameters);
    }
    
      public ScipionGalleryJFrame(ScipionGalleryData data) {
        super(data);
        readScipionParams((ScipionParams)data.parameters);
    }

    protected void readScipionParams(ScipionParams parameters)
    {
        try {
            type = ((ScipionGalleryData)data).getScipionType() + "s";
            python = parameters.python;
            script = parameters.scripts + File.separator + "pw_create_image_subset.py";
            ctfscript = parameters.scripts + File.separator + "pw_recalculatectf.py";
            projectid = parameters.projectid;
            inputid = parameters.inputid;
            outputdir = new File(data.getFileName()).getParent();
            sqlitefile = String.format("%s%sselection%s", outputdir, File.separator, data.getFileExtension());
            msgfields = new HashMap<String, String>();
            msgfields.put(runNameKey, "ProtUserSubset");
            other = parameters.other;
            initComponents();
        } catch (Exception ex) {
            Logger.getLogger(ScipionGalleryJFrame.class.getName()).log(Level.SEVERE, null, ex);
            throw new IllegalArgumentException(ex.getMessage());
        }
    }
    
    
    
    private void initComponents() {
        JButton closebt = XmippWindowUtil.getTextButton("Close", new ActionListener() {

            @Override
            public void actionPerformed(ActionEvent ae) {
                close();
            }
        });
        buttonspn.add(closebt);
        if (type != null) {
            cmdbutton = getScipionButton("Create " + type, new ActionListener() {

                @Override
                public void actionPerformed(ActionEvent ae) {
                    int size;
                    MetaData imagesmd = null;    
                    if(data.hasClasses())
                        imagesmd = data.getEnabledClassesImages();
                    else
                        imagesmd = data.getMd(data.getEnabledIds());
                    size = imagesmd.size();
                    String question = String.format("<html>Are you sure you want to create a new set of %s with <font color=red>%s</font> %s?", type, size, (size > 1)?"elements":"element");
                    ScipionMessageDialog dlg = new ScipionMessageDialog(ScipionGalleryJFrame.this, "Question", question, msgfields);
                    int create = dlg.action;
                    if (create == ScipionMessageDialog.OK_OPTION) 
                    {
                        String[] command = new String[]{python, script, projectid, inputid, sqlitefile + "," + ((ScipionGalleryData)data).getPrefix(), String.format("SetOf%s", type), dlg.getFieldValue(runNameKey), other};
                        runCommand(command);
                    }
                }
            });
            if(data.hasClasses())
            {
                classcmdbutton = getScipionButton("Create Classes", new ActionListener() {

                    @Override
                    public void actionPerformed(ActionEvent ae) {
                        MetaData md = data.getMd(data.getEnabledIds());
                        int size = md.size();
                        String msg = String.format("<html>Are you sure you want to create a new set of Classes with <font color=red>%s</font> %s?", size, (size > 1)?"elements":"element");
                        ScipionMessageDialog dlg = new ScipionMessageDialog(ScipionGalleryJFrame.this, "Question", msg, msgfields);
                        int create = dlg.action;
                        if (create == ScipionMessageDialog.OK_OPTION) {
                            String output = ((ScipionGalleryData)data).getSelf().equals("Class2D")? "SetOfClasses2D":"SetOfClasses3D";
                            String[] command = new String[]{python, script, projectid, inputid, sqlitefile + "," + ((ScipionGalleryData)data).getPrefix(), output , dlg.getFieldValue(runNameKey), other};
                            runCommand(command);
                            
                        }

                    }
                });
                
                buttonspn.add(classcmdbutton);
            }
            
            buttonspn.add(cmdbutton);
            if(data.isCTFMd())
            {
                JButton recalculatectfbt = getScipionButton("Recalculate CTFs", new ActionListener() {

                    @Override
                    public void actionPerformed(ActionEvent ae) {
                        String recalculatefile = outputdir + File.separator + "ctfrecalculate.txt";
                        ((ScipionGalleryData)data).exportCTFRecalculate(recalculatefile);
                        String[] command = new String[]{python, ctfscript, projectid, inputid, recalculatefile};
                        runCommand(command);
                    }
                });
                buttonspn.add(recalculatectfbt);
            }
            pack();
            enableActions();
            jcbBlocks.addActionListener(new ActionListener() {

                @Override
                public void actionPerformed(ActionEvent ae) {
                    enableActions();
                }
            });
        }

    }

    public void reloadTableData(boolean changed)
    {
        super.reloadTableData(changed);
        enableActions();
    }
    

    public JButton getScipionButton(String text, ActionListener listener) {

        JButton button = new JButton(text);
        button.addActionListener(listener);
        button.setBackground(ScipionMessageDialog.firebrick);
        button.setForeground(Color.WHITE);
        return button;
    }

    

    protected void enableActions() {
        boolean isenabled = data.allowGallery();
        Color color = isenabled ? ScipionMessageDialog.firebrick : ScipionMessageDialog.lightgrey;
        Color forecolor = isenabled ? Color.WHITE : Color.GRAY;
        if(cmdbutton != null)
        {
            cmdbutton.setEnabled(isenabled);
            cmdbutton.setBackground(color);
            cmdbutton.setForeground(forecolor);
        }
        if(classcmdbutton != null)
        {
            isenabled = data.hasClasses() && !data.isVolumeMode();
            color = isenabled? ScipionMessageDialog.firebrick: ScipionMessageDialog.lightgrey; 
            forecolor = isenabled? Color.WHITE: Color.GRAY;
            classcmdbutton.setEnabled( isenabled);
            classcmdbutton.setBackground(color);
            classcmdbutton.setForeground(forecolor);
        }
    }

  
    	public boolean proceedWithChanges()
	{
            return true;//without asking for changes
	}
    
   protected void runCommand(final String[] command) 
    {
        XmippWindowUtil.blockGUI(ScipionGalleryJFrame.this, "Creating set ...");
        new Thread(new Runnable() {

            @Override
            public void run() {

                try {
                    ((ScipionGalleryData)data).overwrite(sqlitefile);
                    String output = XmippUtil.executeCommand(command);
                    XmippWindowUtil.releaseGUI(ScipionGalleryJFrame.this.getRootPane());
                    if (output != null && !output.isEmpty()) {
                        System.out.println(output);
                        XmippDialog.showInfo(ScipionGalleryJFrame.this, output);
                        
                    }

                } catch (Exception ex) {
                    throw new IllegalArgumentException(ex.getMessage());
                }

            }
        }).start();
    }
        


}
