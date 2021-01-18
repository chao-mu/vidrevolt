ChaosMonkey = {}
function ChaosMonkey.new(rand_stack)
    local self = {}

    self.destinations = {}

    self.pipeline = {}

    self.rand_stack = rand_stack or {}
    self.rand_index = 1

    self.write_stats = {}
    self.read_stats = {}

    function self:random()
        if self.rand_index > #self.rand_stack then
            table.insert(self.rand_stack, rando())
        end

        local rand = self.rand_stack[self.rand_index]
        self.rand_index = self.rand_index + 1

        return rand
    end

    function self:random_element(arr)
        return arr[math.floor(#arr * self:random() + 1)]
    end

    function self:delete_dest(k)
        local indices = {}
        for i, v in pairs(self.destinations) do
            if v == k then
                table.insert(indices, i)
            end
        end

        for i, target in pairs(indices) do
            table.remove(self.destinations, target)
            self.write_stats[target] = nil
        end
    end

    function self:new_dest()
        return self:dest("ChaosMonkey:random:" .. self:random())
    end

    function self:add_rend(dest, shader, param_names)
        local inputs = {}
        for i, param_name in ipairs(param_names) do
            inputs[param_name] = self:src()
        end

        table.insert(self.pipeline, {dest=dest, shader=shader, inputs=inputs})

        return dest
    end

    function self:add_copy_into(dest, src)
        table.insert(self.pipeline, {dest=dest, shader="shaders/pass.glsl", inputs={img0=src}})
    end

    function self:rend(dest, shader, param_names)
        self:add_rend(dest, shader, param_names)
        rend(dest, shader, self.pipeline[#self.pipeline].inputs)
    end

    function self:dest(k)
        if k == nil then
            k = self:random_element(self.destinations)
        end

        if self.write_stats[k] == nil then
            self.write_stats[k] = 0
            table.insert(self.destinations, k)
        end

        self.write_stats[k] = self.write_stats[k] + 1

        return k
    end

    function self:src()
        local k = self:random_element(self.destinations)

        if self.read_stats[k] == nil then
            self.read_stats[k] = 0
        end

        self.read_stats[k] = self.read_stats[k] + 1

        return k
    end


    function self:start()
--        print("==READ STATS==")
--        for k, v in pairs(self.read_stats) do
--            print(k .. " " .. v)
--        end

        self.pipeline = {}
        self.rand_index = 1
        self.write_stats = {}
        self.read_stats = {}
        self.destinations = {}
    end

    function self:reseed()
        self.rand_stack = {}
        self.rand_index = 1
    end

    function self:fingerprint()
        local total = 0
        for i, x in ipairs(self.rand_stack) do
            total = total + x
        end

        return total
    end

    return self
end
