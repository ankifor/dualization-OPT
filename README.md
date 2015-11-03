# dualization-OPT
#branch RUNC-M

##Preleminaries
Dualization is one of the central enumeration problems in discrete mathematics. 

Let <i>L = ||a<sub>ij</sub>||<sub>m&#215;n</sub></i> be a Boolean matrix and <i>H</i> be a set of columns of <i>L</i>.
The set <i>H</i> is called a <i>covering</i> of <i>L</i> if each row of <i>L</i> has at least one unit element in the columns <i>H</i>.
A covering <i>H</i> is called <i>irreducible</i> if any proper subset of <i>H</i> is not a covering of <i>L</i>.
Let <i>P(L)</i> denote the set of all possible irreducible coverings of <i>L</i>.
The task is to construct <i>P(L)</i>.

This repository contains an implementation of dualization algorithm RUNC-M (Djukova E.V., Prokofyev P.A. Asymptotically Optimal Dualization Algorithms. //
Computations mathematics and mathematical physics, 2015. Vol. 55,  5, pp. 891 - 905) which is considered to be one of the fastest.

##Contents
<ul>
  <li>
    Folder <b>OPT</b> contains sources for sequential version of RUNC-M. The program has command line interface and should be run with the following arguments: "&lt;input_file&gt; &lt;output_file&gt;", where &lt;input_file&gt; should be a file containing boolean matrix and &lt;output_file&gt; is a file where irreducible coverings of boolean matrix are enumerated (actually, it could be set as NUL in Windows and /dev/null in Linux).The extension of &lt;input_file&gt; defines the format and can be one of the following:
    <ul>
      <li>
      ".bm" - binary matrix format. Elements in each row are represented by 0's and 1's. Example:<br>
      1 0 1 <br>
      1 1 0 <br>
      1 1 1
      </li>
      <li>
      ".hg" - hypergraph format. Units in each row are represented by indicies. Example for the same matrix:<br>
      1 3<br>
      1 2<br>
      1 2 3
      </li>
      <li>
      ".packed" - an internal binary format (see utility). 
      </li>
    </ul>
  </li>
  <br>
  <li>
  Folder <b>OPT_Parallel</b> contains an MPI parallel version of RUNC-M. To compile it one should add folder OPT as "include directory" and MPI include directory. Required MPI libs (msmpi.lib in my case) should be also specified. Command line arguments: "&lt;input_file&gt; &lt;output_file&gt; &lt;id&gt; &lt;method related args&gt;". Argument &lt;id&gt; is used for printing statistics about current run to stdout (&lt;id&gt; &lt;method_num&gt; &lt;current process num&gt; &lt;world_size&gt; &lt;number of found coverings by process&gt; &lt;expected load per processor&gt; &lt;time consumed for task size estimation&gt; &lt;time consumed by process for dualization&gt;). Method related args can be the following:
    <ul>
      <li>
      "stripe &lt;u&gt; &lt;times&gt;" - so called stripe method. Assume that <i>L</i> is an <i>m-by-n</i> matrix. Then <times> instances of <i>u-by-n</i> submatrices of <i>L</i> are processed in order to estimate computational task sizes. 
      </li>
      <li>
      "uniform" - all computational task sizes are assumed to be equal.
      </li>
    </ul>
  </li>
  <br>
  <li>
  Folder <b>Utilities</b> contains some utilies for dualization. To compile it one should add folder OPT as "include directory". The arguments can be the following:
    <ul>
      <li>
      "-random &lt;output_file&gt; &lt;m&gt; &lt;n&gt; &lt;prob&gt; &lt;seed&gt;" - generate random <i>m-by-n</i> boolen matrix with the probaility of unit equal to &lt;prob&gt;. Argument &lt;seed&gt; should be an integer between 0 and 2^15-1. THe extension of &lt;output_file&gt; defines the output format (".bm", ".hg" or ".packed"). 
      </li>
      <li>
      "-sort &lt;input_file&gt; &lt;output_file&gt;" takes matrix specified in &lt;input_file&gt; with valid extension (".bm", ".hg" or ".packed"), sorts its columns in descending order of number of units in a column and prints it to &lt;output_file&gt;  with valid extension (".bm", ".hg" or ".packed").
      </li>
      <li>
      "-convert &lt;input_file&gt; &lt;output_file&gt;" converts matrix file from one format to another. Valid extensions should be specified (".bm", ".hg" or ".packed").
      </li>
    </ul>  
  </li>
</ul>

