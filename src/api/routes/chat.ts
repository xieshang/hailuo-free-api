import _ from 'lodash';

import Request from '@/lib/request/Request.ts';
import Response from '@/lib/response/Response.ts';
import core from '../controllers/core.ts';
import chat from '@/api/controllers/chat.ts';
import logger from '@/lib/logger.ts';
import audio from '@/api/controllers/audio.ts';
import fs from "fs-extra";

export default {

    prefix: '/v1/chat',

    post: {

        '/completions': async (request: Request) => {
            request
                .validate('body.conversation_id', v => _.isUndefined(v) || _.isString(v))
                .validate('body.messages', _.isArray)
                .validate('headers.authorization', _.isString)
            // token切分
            const tokens = core.tokenSplit(request.headers.authorization);
            // 随机挑选一个token
            const token = _.sample(tokens);
            const { model, conversation_id: convId, messages, stream } = request.body;
            if (stream) {
                const stream = await chat.createCompletionStream(model, messages, token, convId);
                return new Response(stream, {
                    type: "text/event-stream"
                });
            }
            else
                return await chat.createCompletion(model, messages, token, convId);
        },
        
        "/phone_msg": async (request: Request) => {
        request
          .validate("body.model", _.isString)
          .validate("body.response_format", v => _.isUndefined(v) || _.isString(v))
          .validate("headers.authorization", _.isString);
        // token切分
        const tokens = core.tokenSplit(request.headers.authorization);
        // 随机挑选一个token
        const token = _.sample(tokens);
        // logger.info("tokens:", tokens);
        // logger.info("token:", token);
        if(!request.files['file'] && !request.body["file"])
          throw new Error('File field is not set');
        let tmpFilePath;
        if(request.files['file']) {
          const file = request.files['file'];
          if(!['audio/mp3', 'audio/mpeg', 'audio/x-wav', 'audio/wave', 'audio/mp4a-latm', 'audio/flac', 'audio/ogg', 'audio/webm'].includes(file.mimetype))
            throw new Error(`File MIME type ${file.mimetype} is unsupported`);
          tmpFilePath = file.filepath;
          logger.info("tmpFilePath：",tmpFilePath);
        }
        else
          throw new Error('File field is not set');
        const { model, response_format: responseFormat = 'json' } = request.body;
        const binary = await audio.createPhomeMsg(model, tmpFilePath, token);
        return new Response(binary, {
          type: "audio/wav",
          headers: {
            "Content-Disposition": "attachment; filename=phone_msg.wav"
          },
        });
      },
    },
};