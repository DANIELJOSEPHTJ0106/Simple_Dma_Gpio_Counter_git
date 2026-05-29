# build.tcl
# Script to build the Simple_Dma_Gpio_Counter project headlessly in Vivado

puts "================================================="
puts " Starting automated build process...             "
puts "================================================="

# 1. Open the existing project
puts "\n---> Opening project..."
open_project Simple_Dma_Gpio_Counter_git.xpr

# 2. Reset any previous runs
puts "\n---> Resetting previous runs..."
reset_run synth_1
reset_run impl_1

# 3. Launch Synthesis and wait for completion
puts "\n---> Launching Synthesis..."
launch_runs synth_1 -jobs 8
wait_on_run synth_1

# Check if Synthesis failed
if {[get_property PROGRESS [get_runs synth_1]] != "100%"} {
    puts "ERROR: Synthesis failed."
    exit 1
}

# 4. Launch Implementation and Bitstream Generation, and wait
puts "\n---> Launching Implementation and Bitstream Generation..."
launch_runs impl_1 -to_step write_bitstream -jobs 8
wait_on_run impl_1

# Check if Implementation failed
if {[get_property PROGRESS [get_runs impl_1]] != "100%"} {
    puts "ERROR: Implementation failed."
    exit 1
}

# 5. Export Hardware (including bitstream) to .xsa file
puts "\n---> Exporting hardware to design_1_wrapper.xsa..."
write_hw_platform -fixed -include_bit -force -file design_1_wrapper.xsa

puts "\n================================================="
puts " Build completed successfully!                   "
puts "================================================="
