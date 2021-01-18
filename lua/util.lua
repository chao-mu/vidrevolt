time_params = {}
function t(key, speed)
    if time_params[key] ~= nil then
        time_params[key] = time_params[key] + time_delta * speed
    else
        time_params[key] = 0
    end

    return time_params[key]
end

function reset_t(key)
    time_params[key] = 0
end

function value(input, amp, shift)
    amp = amp or 1
    input = input or 1
    shift = shift or shift

    return {input=input, amp=amp, shift=shift}
end

function shuffle(tbl)
  math.randomseed(os.time())
  for i = #tbl, 2, -1 do
    local j = math.random(i)
    tbl[i], tbl[j] = tbl[j], tbl[i]
  end
  return tbl
end

function next_index(i, diff, arr)
    local new = i + diff - 1
    if new < 0 then
        new = new + #arr
    end

    new = new % #arr

    return new + 1
end

function copy_into(dest, source)
    rend(dest, "shaders/pass.glsl", { img0=source })
end

function print_rend_call(call)
    local inputs = ""
    for name, value in pairs(call.inputs)  do
        inputs = inputs .. "    " .. name .. '="' .. value .. '",\n'
    end

    print('rend("' .. call.dest .. '", "' .. call.shader .. '", {\n' .. inputs .. '})\n')
end

function rend_call_stack(calls)
    for i, call in ipairs(calls) do
        rend(call.dest, call.shader, call.inputs)
    end
end
