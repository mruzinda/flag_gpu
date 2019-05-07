function [ index ] = input_index(m,f,t,c,e)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here
%N_INPUTS_PER_FENGINE = 6;

Nc = 8;
Nt = 85;
Nf = 3;
Ne = 6;
% Nc = 25;
% Nt = 20;
% Nf = 5;

% index = 2*(c+Nc*(t+Nt*(f+Nf*m)));    %sizeof(uint64_t)
index = 2*(e+Ne*(c+Nc*(t+Nt*(f+Nf*m))));

end


