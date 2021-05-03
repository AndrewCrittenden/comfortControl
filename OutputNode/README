# Purpose
The output node is designed to use the desired thermal power sent by the centralized interface to control the thermoelectric appliance inside the minifridge. 

## Control Method
The appliance is controlled by comparing the desired heat delivered to the fridge to the heat actually delivered to the fridge. The desired heat is found by multiplying 
the desired thermal power by the length of one data cycle. The heat actually delivered is found by multiplying the change in ambient temperature over the last data cycle
by the thermal mass of the air inside the fridge. When the desired heat is met, the appliance will turn off. When more heating or cooling is needed, the appliance will 
run at max heating or cooling respectively.

## Thermoelectric Appliance
The thermoelectric appliance is rather difficult to control, but the poor thermal conduction of the fridge allowed a very simple control method to be used. There are three
coefficients that describe thermoelectric behavior: the Seebeck coefficient, module resistance, and thermal conductance. Each of these coefficients vary based on the 
temperature of both sides of the appliance. In the simplified control method, they are only used to check for a special case of cooling. The appliance can reach a state 
where max cooling power does not result in max cooling inside the appliance. This condition is checked using the Iprime variable in the control algorithm.
