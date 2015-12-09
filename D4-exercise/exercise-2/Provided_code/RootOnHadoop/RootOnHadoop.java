package cern.root.hadoop;

// Java
import java.io.IOException;
import java.util.*;
import java.io.*;
import java.net.InetAddress;
import java.net.UnknownHostException;

 // Hadoop 
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.conf.*;
import org.apache.hadoop.io.*;
import org.apache.hadoop.mapred.*;
import org.apache.hadoop.util.*;

// My custom Hadoop input format
import cern.root.hadoop.WholeFileInputFormat;


public class RootOnHadoop extends Configured implements Tool {
  
 
    // Support function to exec shell commands
    static String execCmd(String command) {
        
        System.out.println("execCmd: Executing \""+command+"\"");
    
        String output="";
    
        try {  
            Process p = Runtime.getRuntime().exec(command);
	      
            // Print stdout 
            BufferedReader in = new BufferedReader(  
                                  new InputStreamReader(p.getInputStream()));  
            String line = null;  
            while ((line = in.readLine()) != null) {  
                output = output+line+"\n";
            }
              
            // Print stderr
            BufferedReader err = new BufferedReader(  
                                  new InputStreamReader(p.getErrorStream()));  
            String lineErr = null;  
            while ((lineErr = err.readLine()) != null) {  
                output = output+lineErr+"\n";
            }
                
        } catch (IOException e) {  
            e.printStackTrace();  
        }
        
        return output;
        
    }    
  
  

  
  
  
     /*****************************************************/
    /*                     Mapper                        */
    /*****************************************************/
  
    static class RootOnHadoopMapper extends MapReduceBase
    implements Mapper<Text, BytesWritable, IntWritable, Text> {
    
        private JobConf conf;
        private static final IntWritable one = new IntWritable(1);


        @Override
        public void configure(JobConf conf) {
            this.conf = conf;
        }


        @Override
        public void map(Text key, BytesWritable value,
            OutputCollector<IntWritable, Text> output, Reporter reporter)
            throws IOException {
           
            // Get some vars from the Job's configuration
            String job_id = conf.get("mapred.job.id");
         
            String outputdir = conf.get("mapred.output.dir");
    
            String filename = conf.get("map.input.file");
         
            String map_id = conf.get("mapred.task.partition");
          
            String hdfs_on_local_fs= conf.get("dfs.data.dir");
            // Example value: /data01/hadoop/dfs/data,/data02/hadoop/dfs/data
          
            // Map script name
            String [] mapName_pieces=conf.get("RootOnHadoop.mapName").split("/");
            String mapName = mapName_pieces[mapName_pieces.length-1];
          
            // Here we use a fake keyword, the "one".
            output.collect(one, new Text(map_id));
 
            // Parse and build some paths...        
            String[] outputdir_pieces = outputdir.split("/");
            String outputdir_basename = outputdir_pieces[outputdir_pieces.length-1];
    
            String[] arr = filename.split("/");
            String basefilename = arr[arr.length-1];     
    
            String dfsfilename = "";
            String[] arr1 = filename.split("/");
            for (int i = 3; i < arr1.length; i++) {	  
    	        dfsfilename = dfsfilename + "/" + arr1[i];
            }
    

            try {
            
                // Create file for the stdout and stderr output
                FileWriter fstream = new FileWriter("./map.out");
                BufferedWriter out = new BufferedWriter(fstream);

                out.write("===== The following is the RootOnHadoop output =====\n");

                // Get the Map script from the temp directory
                out.write(execCmd("hadoop fs -get "+outputdir+"/../"+outputdir_basename+"_tmp/"+mapName+" ./"));
            
                // Make it executable
                out.write(execCmd("chmod 755 ./"+mapName));


                // ============================================ //
                // Core part: how to access the input data file //
                // ============================================ //
                
                // The "file" string contains the file that this Java Map task is in charge of processing. 
                // First, remove the the hostname from it (it is in the form hdfs://namenode/file)

                String file=filename;
                String [] full_path_pcs= file.split("/");
        
                file="";        
                for (int j = 3; j < full_path_pcs.length ; j++) {       
                    file+="/"+full_path_pcs[j];       
                }
        
                // Ok, we have the fine name on HDFS
                out.write("Working on " + file +"\n");
    
                // Vars..
                String file_path="";
                boolean local_not_found=true;
    
                // Obtain informations on the blocks and on the location of the replicas for this file
                String result = execCmd("hadoop fsck "+file+" -files -blocks -locations");

                String[] pieces=result.split("\n");

                // Check that we have only one block:
                int line =0;
                boolean found=false;
                while (!found && line < pieces.length) {
                    if(0<=pieces[line].indexOf("Total blocks")) {
                        found=true;
    			    }
			    
                    if (!found) line++;	
                }
                
                if (!found) {
                    out.write("Error while reading hadoop fsck output to obtain the numer of blocks\n");
                    System.exit(1);
                }
		 
		      
                // Example of output expected: "Total blocks (validated):	1 (avg. block size 55063110 B)"
                String num_blocks;
                num_blocks=pieces[line].split("	")[1];
                num_blocks=num_blocks.split(" ")[0];

		 
                if (num_blocks.equals("1")) {

                    out.write("Ok, file is made of only one block\n");
    		 
    		 
                    // Is a local replica available on this node?
                    // Example line to parse: "0. blk_1053905684496398771_565267 len=55063110
                    // repl=2 [128.142.172.104:1004, 128.142.172.101:1004]"

                    // Search for the correct line to parse and save its index in the "line" var
                    found=false;
                    line=0;
    		      
                    while (!found && line < pieces.length) {
                        if ( (0<=pieces[line].indexOf("len=")) &&
        		           (0<=pieces[line].indexOf("repl=")) && 
        		           (0<=pieces[line].indexOf("[")) ) {
                            found=true;
                            // Debug: out.write (pieces[line]+"\n");
                            }
                        if (!found) line++;	
        		    }
        		    
                	// Obtain the IP address of this node:        
            		InetAddress ip;
            		String node_ip="";
                    try { 
               			ip = InetAddress.getLocalHost();
               		    node_ip=ip.getHostAddress();
               			out.write("Node IP address: "+ node_ip+"\n");
                    } catch (UnknownHostException e) {  
            			e.printStackTrace();
            		}
            
        		    
                    // Parse the line to obtain the IP addresses of the replicas
                    String replica_ips_str;
                    replica_ips_str = pieces[line].split("\\[")[1];
                    replica_ips_str = replica_ips_str.split("\\]")[0];
                    String [] replica_ips = replica_ips_str.split(", ");
                	 
                        // For evry replica
                        for (int i = 0; i < replica_ips.length; i++) {    
                        
                            // Cut the port info and keep the IP address 	        
                	        replica_ips[i]=replica_ips[i].split(":")[0];
                	        out.write("Replica IP address: "+replica_ips[i]+"\n");
                	        
                	        // Check if the replica's IP address is the same as the node's one
                	        if (replica_ips[i].equals(node_ip)) {
                                out.write("Ok, proceed with the local access\n");
                                local_not_found=false;
                                
                                // I we are here it means that a local replica is available,
                                // use one of the two following methods to access it.
                	           
                	            int method=2;
                	           
                	            if (method==1) {
                	           
                    	            // 1) FUSE: is safe but introduces some latency. In my testes,
                    	            // it's about 15-20% slower than the following (DirectAccess) method
                    	           
                    	            String fuse_mountpoint="/hdfs";
                    	               	           
                    	            file_path=fuse_mountpoint+file;
                	            
                                }
                                
                                else {
                	           
                                    // 2) Direct access: requires to grant access permission to the user
                                    // executing the MapRedcue Job to  HDFS's data dir
                                    //
                                    // (i.e.: chmod from 700 to 755 /data01/hadoop/dfs/) on every node.
                                    //
                	                // This, with a proper MapReduce job, can of course break HDFS acls and is
                	                // therefore not suitable for shared cluster where users store sensible data.    	           
                	            
    
                                    // Locate the corresponding block file on the local file system
                               
                                    // Example line to parse: "0. blk_1053905684496398771_565267 len=55063110
                                    // repl=2 [128.142.172.104:1004, 128.142.172.101:1004]"
    
                                    String block_id=pieces[line].split(" ")[1];
                                    block_id="blk_"+block_id.split("_")[1];
                               
                                    out.write("block_id:" +block_id +"\n");
                               
                                    String [] hdfs_on_local_fs_paths=hdfs_on_local_fs.split(","); 
                	           
                	                String block_local_path="";
                	           
                	                String location="";
                	           
                	                for (int k = 0; k < hdfs_on_local_fs_paths.length; k++) {
                	           
                                    String find_out=execCmd("find "+hdfs_on_local_fs_paths[k]+"/current/ -name "+block_id);
                	                out.write("find on" +hdfs_on_local_fs_paths[k] + ": " + find_out +"\n");
                	                 
                	           	    location+=find_out;
                	           	  
    
                                }
                                
                                // Remove the new line char at the end
                                location=location.substring(0, location.length()-1);
                                
                                out.write("Input file location="+location+"\n");
                                
                                // Check if we found only one block:
                                if(location.indexOf("\n")<0){
                                
                                    file_path=location;
                                    // else the file_path var stays blank causing 
                                    // to switch on the "hadoop fs -get" access method
                                }
                                
                                // Exit the loop if we found a local replica available.
                                break;
                                
                            }
                     	           
                        }
        	        
                    }
                    
                }
                
                
                else {
                    out.write("File made of more than one block, aborting direct access\n"); }
    
    
                // Check the situation
                if (local_not_found || file_path.equals("")) {
            
                    // If we are here it means that the file is made
                    // of more than one block or that a local replica is unavailable
                      
                    out.write("No local replica found, proceed with the remote copy\n");
                
                    // Copy the file here (we are in the Map's sandobx) and then set the file path                 
                    out.write(execCmd("hadoop fs -get "+file+ " ./"));
                    
                    // Obtain the file name                  
                    String [] file_name_pieces=file.split("/");
                    String file_name=file_name_pieces[file_name_pieces.length-1];  
                    file_path=System.getProperty("user.dir")+"/"+file_name;

                } 
        

                out.write("====================================================\n");

                out.write("===== The following is your Map task output =====\n");

                
                // Run the Map script on the file that this map task hasto process!
                out.write(execCmd("./"+mapName+" " + file_path));
                
                out.write("=================================================\n");
                
                // Close output stream
                out.close();
                
                // Upload every file which ends in ".out" to "mapNumber.filename.out"
                File dir = new File("./");
                for (File fileItem : dir.listFiles()) {
                    if (fileItem.getName().endsWith((".out"))) {
                        execCmd("hadoop fs -put "+fileItem.getName()+" "+outputdir+"/"+ map_id+"."+fileItem.getName());                 
                    }
                }
               
            } catch (Exception e){
                //Catch exception if any
                System.err.println("Error: " + e.getMessage());    
            }
            
        // End of the map function
        }
        
    // End of RootOnHadoopMapper class        
    }





    /*****************************************************/
    /*                     Reducer                       */
    /*****************************************************/

    static class RootOnHadoopReducer extends MapReduceBase
    implements Reducer<IntWritable, Text, Text, NullWritable> {
        
        private JobConf conf; 
        private static final NullWritable n = NullWritable.get();
        
          
        @Override
        public void configure(JobConf conf) {
            this.conf = conf;
        }
    
        @Override
        public void reduce(IntWritable key, Iterator<Text> values,
        OutputCollector<Text, NullWritable> output,
        Reporter reporter) throws IOException {
     
            // Get some vars from the Job's configuration
            
            String outputdir = conf.get("mapred.output.dir");
                 
            // Reduce name
            String [] reduceName_pieces=conf.get("RootOnHadoop.reduceName").split("/");
            String reduceName = reduceName_pieces[reduceName_pieces.length-1];
            
            // Parse and build some paths...         
            String[] outputdir_pieces = outputdir.split("/");
            String outputdir_basename = outputdir_pieces[outputdir_pieces.length-1];


            // Create output file 
            FileWriter fstream = new FileWriter("./reduce.out");
            BufferedWriter out = new BufferedWriter(fstream);

            out.write("===== The following is the RootOnHadoop output =====\n");

            // Get the Map script from the temp directory
            out.write(execCmd("hadoop fs -get "+outputdir+"/../"+outputdir_basename+"_tmp/"+reduceName+" ./"));

            // Make it executable
            out.write(execCmd("chmod 755 ./"+reduceName));
        
            out.write("===== The following is your Reduce task output =====\n");
     
     
            // Start the Reduce script
            
            try{

                String line;
                OutputStream stdin = null;
                InputStream stderr = null;
                InputStream stdout = null;

                // launch the Reduce script and grab stdin/stdout and stderr
                Process process = Runtime.getRuntime ().exec ("./"+reduceName +" "+outputdir);
                stdin = process.getOutputStream ();
                stderr = process.getErrorStream ();
                stdout = process.getInputStream ();


                // Thanks to the fake "one" key, for every Map task which ends,
                // we have its ID as an item in the "values" array 
                while (values.hasNext()) {
    
                    String value = values.next().toString(); 
    		 
                    // Feed the stdin of the Reduce script with the ID of the map tasks
                    line = value + "\n";
                    stdin.write(line.getBytes() );
                    stdin.flush();

                }
                
                // Close the stdin (and send EOF charachter)
                stdin.close();

                // Clean up if any output in stdout
                BufferedReader brCleanUp = new BufferedReader (new InputStreamReader (stdout));
                
                // Write the Reduce script output to the output file
                while ((line = brCleanUp.readLine ()) != null) {
                
                    out.write(line+"\n");
                                   
                    // If you want the collect the output of the Reduce using the Hadoop MapReduce framework use:    
                    // output.collect(new Text(line+"\n"),n);
                }
                               
                brCleanUp.close();
            }
            catch (Exception err) {
                err.printStackTrace();
            }
           
            out.write("=================================================\n");            
	
            out.close();
            
            // Upload the Reduce output to the Job's output dir
            execCmd("hadoop fs -put ./reduce.out " + outputdir + "/reduce.out");

        // End ot the reduce function
        }
    
    // End of the RootOnHadoopReducer class
    }



 


    // The run method, which is invoked when launching the Job
    @Override
    public int run(String[] args) throws IOException {
        JobConf conf = new JobConf(getConf(), getClass());
        if (conf == null) {
            return -1;
        }
        
        boolean mapSet=false;
        boolean reduceSet=false;
        boolean inSet=false;
        boolean outSet=false;
        String input="";
        String output="";
        
        // Parse comand line arguments
        for (int j = 0; j < args.length; j++) {
        
            if (args[j].equals("-map")){
                conf.set("RootOnHadoop.mapName", args[j+1]);
                mapSet=true;
                j++;
            }
                
            if (args[j].equals("-reduce")){
                conf.set("RootOnHadoop.reduceName", args[j+1]);
                reduceSet=true;
                j++;
            }
            
            if (args[j].equals("-in")){
                input=args[j+1];
                inSet=true;
                j++;
            }
            
           if (args[j].equals("-out")){
                output=args[j+1];
                outSet=true;
                j++;
            }
                
         }
        
        
        if (!mapSet || !reduceSet || !inSet || !outSet){ 
            System.out.println("Usage: -map your_map -reduce your_reduce -in input -out output");
            System.exit(1);
        }
        
      
        // Set to use the WholeFileInputFormat class.
        // In this way even if you stored a binary ROOT input file in more than one chunk, you will have only Map
        // task per file and the task will not fail. But you won't take advantage of data locality. 
        conf.setInputFormat(WholeFileInputFormat.class);
        WholeFileInputFormat.addInputPath(conf, new Path(input));
    
        // Use plain text output from Map to Reduce
        conf.setOutputFormat(TextOutputFormat.class);
        TextOutputFormat.setOutputPath(conf, new Path(output));

        conf.setOutputKeyClass(IntWritable.class);
        conf.setOutputValueClass(Text.class);
    
        conf.setMapperClass(RootOnHadoopMapper.class);
        //conf.setReducerClass(IdentityReducer.class);
        conf.setReducerClass(RootOnHadoopReducer.class);
    
        // Only one reducer
        conf.setNumReduceTasks(1);
        
        // Get basename of the output dir basename
        String outputdir = conf.get("mapred.output.dir");
        String[] outputdir_pieces = outputdir.split("/");
        String outputdir_basename = outputdir_pieces[outputdir_pieces.length-1];
     
        // Create a temp dir
        System.out.println("Creating temp dir "+outputdir+"/../"+outputdir_basename+"_tmp ...");
        System.out.println(execCmd("hadoop fs -mkdir "+outputdir+"/../"+outputdir_basename+"_tmp"));
        
        // Upload Mapper and Reducer to the temp dir
        System.out.println("Uploading map task ...");
        
        System.out.println(execCmd("hadoop fs -put "+conf.get("RootOnHadoop.mapName")+" "+outputdir+"/../"+outputdir_basename+"_tmp/"));
        
        System.out.println("Uploading reduce task ...");             
        System.out.println(execCmd("hadoop fs -put "+conf.get("RootOnHadoop.reduceName")+" "+outputdir+"/../"+outputdir_basename+"_tmp/"));
  
        // Run the MapReduce Job
        JobClient.runJob(conf);
           
        // Remove the temp dir
        System.out.println("Removing temp dir ...");
        System.out.println(execCmd("hadoop fs -rmr "+outputdir+"/../"+outputdir_basename+"_tmp"));
        
        return 0;
        }
  

    // Main ...
    public static void main(String[] args) throws Exception {

        int exitCode = ToolRunner.run(new RootOnHadoop(), args);
        System.exit(exitCode);
    }
    
    
}











