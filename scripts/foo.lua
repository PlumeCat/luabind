-- foo.lua

require "bar"

function foo2(a, b, c)
	print("FOO2", a, b, c)
	return nil, 4
end

print("lambda; ", lambda(3, 0.125))
t = returns_vec()

for i, v in ipairs(t) do print(i, v) end



examples = {}

function add_example(e)
	examples[e] = example_name(e)
end

function foo1()

	for i, e in pairs(examples) do
		print(i, e)
	end

end
