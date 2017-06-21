# OrderBook

This is an implementation of an order book that I created in the process of being interviewed for a quant developer position.

Remarks:
1) In designing the main data structure for the order book class, I focused on fast insert/erase and fast access to the current highest price.  I realise my chosen approach might be overkill for small input files.
2) I've included a number of asserts in the code for the purpose of succinctly documenting some of the assumptions listed in the instructions (for the benefit of anybody reading the code without having the instructions document at hand).
3) On my system I compiled the code using g++ 4.7 and Boost library 1.60.

This is a common programming test for many quant roles.  Please do not embarrass yourself by plagiarising my work as it is easily searchable on Google!
