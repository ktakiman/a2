why:
  1. Assembly language is a bit tedious to program. Don't want to speficy every instruction.
  2. C is a bit too higher lelvel. Language is conversome when trying to precisely control the generated assembly code.
     (i.e. volatile - not very easy to trouble shoot, either do black-box try and error, look at assembly)
  3. Linking is also tedious to finely control the generated assembly code, skipping unused section, import, exports symbols, etc
    - setting up interrupt table properly
    - exclude unused assembly section (uncalled function)
  4. Overall, what needs to be generated (program image) is relatively simple (so far with cortex-m0), but the process to
     generate it seems unnecessary complex, and sometimes not very good (big image size, inefficient use of
     

approach:
  1. Some kind of programming model, which sits a slightly above assembly language.
  2. More direct way of speficying the interrupt table, memory access, etc
  3. Better optimization of generated assembly code

thought:
  1. business vs hobby
    - productivity, efficiency, speed of development < > deep understanding, curiosity, job of making
    - Don't reinvent wheel or Let's reinvent wheel
    - taking shortest path vs taking longest path
    - (when describing this project) don't write top to bottom, sprinkle many ideas with hyperlink

TODO:
  1. Include other .a2 file
  2. Preprocess text, convert tab to spaces, remove training spaces, etc

----------------------------------------------------------------------------------------------------
using gcc on gw/gitbash
----------------------------------------------------------------------------------------------------
- link with standard c++ lib  -lstdc++
- don't use gcc, but use g++ with -std=c++0x

----------------------------------------------------------------------------------------------------
c++ notes to update
----------------------------------------------------------------------------------------------------
- class design, virtual destructor

----------------------------------------------------------------------------------------------------
vim/git
----------------------------------------------------------------------------------------------------
% ~ register holding a current file path
