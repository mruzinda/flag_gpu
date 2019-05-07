function [ index ] = out_index(m,f,t,c,e)
%UNTITLED5 Summary of this function goes here
%   Detailed explanation goes here

Nc = 8;
Nt = 85;
Nf = 4;

% Nc = 25;
% Nt = 20;
% Nf = 5;
Ne = 6;

% index = 2*((f+Nf*(c+Nc*(t+Nt*m))));
index = 2*(e+Ne*(f+Nf*(c+Nc*(t+Nt*m))));

end

