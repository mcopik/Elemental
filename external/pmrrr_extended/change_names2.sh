find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/counter/ext_counter/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/rrr/ext_rrr/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/tasks/ext_tasks/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/odcpy/ext_odcpy/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/odscl/ext_odscl/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/odswap/ext_odswap/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/xdcpy/ext_xdcpy/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/xdscl/ext_xdscl/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/xdswap/ext_xdswap/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/process_c_task/ext_process_c_task/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/plarrv/ext_plarrv/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/process_r_task/ext_process_r_task/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/process_s_task/ext_process_s_task/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/queue/ext_queue/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/plarre/ext_plarre/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/global/ext_global/g" {} \;

find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/pmext_rrr_extended/pmrrr_extended/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/pmext_rrr/pmrrr/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/ext_global.h/global.h/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/ext_plarre.h/plarre.h/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/ext_plarrv.h/plarrv.h/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/ext_counter.h/counter.h/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/ext_rrr.h/rrr.h/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/ext_tasks.h/tasks.h/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/ext_queue.h/queue.h/g" {} \;


find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/pmext_rrr_dscal/pmrrr_dscal/g" {} \;
find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/pmrrr_xscal/pmrrr_xscal/g" {} \;

#find . \( -name \*.c -o -name \*.h \) -exec sed -i "s/old/new/g" {} \;
